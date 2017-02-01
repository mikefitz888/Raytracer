#include "../Include/PhotonMap.h"

namespace photonmap {
	Photon::Photon(glm::vec3 o, glm::vec3 d, unsigned int _depth, float intensity) : beam(o, d), depth(_depth) {
        color *= glm::vec3(intensity);
    }

	void PhotonMapper::mapScene(model::Scene& scene) {
		float total_light_intensity = 0.0f;
		for (auto light : scene.light_sources) { total_light_intensity += light->intensity; }

		unsigned int assigned_photons = 0;
		std::vector<unsigned int> photons_per_light;
		for (int i = 0; i < scene.light_sources.size(); i++) {
			photons_per_light.push_back((unsigned int)(((float)number_of_photons) * (scene.light_sources[i]->intensity / total_light_intensity)));
			assigned_photons += photons_per_light[i];
		}

		photons_per_light[scene.light_sources.size() - 1] += number_of_photons - assigned_photons; //Give any unused photons (due to truncation) to the last light source
		
		/*
        ** http://users.csc.calpoly.edu/~zwood/teaching/csc572/final15/cthomp/index.html
		** At this point the photons have been divided between available light sources, the split being weighted by the intensity of the light source
		** (although I am not sure if this is the best approach. It shouldn't matter for simple examples however.)
		**
		** First step is to generate random directions for each photon based on an appropriate probability density
		** Next step is to simulate the action of the photon throughout the scene, employing russion-roulette approach for randomising photon action
		**   -
		*/

		//Generate photons
		for (int i = 0; i < scene.light_sources.size(); i++) {
			printf("Adding %d photons to light #%d\n", photons_per_light[i], i);
            float total_photons = photons_per_light[i];
			while (photons_per_light[i]--) {
				//Light uniformly distributed in all directions, not necessarily the best approach, we probably want to stop upwards directed light
				auto dir = glm::sphericalRand(1.0f);
				photons.emplace_back(scene.light_sources[i]->position, dir, number_of_bounces, std::pow(2.0f, scene.light_sources[i]->intensity)/(total_photons) );
				//auto pos = scene.light_sources[i]->position;
				//printf("Photon info: Dir=(%f, %f, %f), Pos=(%f, %f, %f), Bounces=%d\n", dir.x, dir.y, dir.z, pos.x, pos.y, pos.z, number_of_bounces);
			}
			printf("Added %d photons to light #%d\n", photons.size(), i);
		}

		/* 
		** Emulate photons
		** Currently handling:
		**	-Global Illumination
		**  -Shadows
		**
		** Planned:
		**  -Refraction, requires russian-roulette style decision making for efficiency
		*/

		Intersection closest, shadow;
		for (auto photon : photons) {
			while (photon.depth-- && photon.beam.closestIntersection(scene.getTrianglesRef(), closest)) { //Until photon misses or reaches bounce limit
                float Pd = 0.2; //Diffuse reflect probability
                float Ps = 0.0; //Specular reflect probabilty
                float p = Rand(); //Generate random number

                if(p <  Pd){ //Diffuse reflect   
                    //Put photon on surface it intersects (absorb).
                    //Bounce photon in random direction with respect to the normal of the surface.
                    //Repeat.
                    glm::vec3 color = closest.color * photon.color;// * glm::vec3(1.0/(1.1-c));
					gathered_photons.emplace_back(photon.color * glm::vec3(1 / sqrt(number_of_bounces - photon.depth)), closest.position, photon.beam.direction);
                    photon.beam.direction = glm::normalize(glm::reflect(photon.beam.direction, scene.getTrianglesRef()[closest.index].getNormal()));
					photon.beam.origin = closest.position + photon.beam.direction * glm::vec3(0.000001);
                }else if(p < Pd + Ps){ //Specular reflect

                }else{ //Absorb photon
                    glm::vec3 color = closest.color * photon.color;// * glm::vec3(1.0/(1.1-c));
					gathered_photons.emplace_back(photon.color * glm::vec3(1 / sqrt(number_of_bounces - photon.depth)), closest.position, photon.beam.direction);
                    break;
                }


				//Store photon information (color/energy, position, direction)
				//float c = closest.color.x>closest.color.y && closest.color.x>closest.color.z ? closest.color.x : closest.color.y>closest.color.z ? closest.color.y : closest.color.z; //Est. max reflectiveness

				/*if (c < Rand() || !photon.depth) {
					//Absorb photon
					glm::vec3 color = closest.color * photon.color * glm::vec3(1.0/(1.1-c));
					gathered_photons.emplace_back(photon.color * glm::vec3(1 / sqrt(number_of_bounces - photon.depth)), closest.position, photon.beam.direction);
				}
				else {
					//Bounce photon
					photon.beam.direction = glm::normalize(glm::reflect(photon.beam.direction, scene.getTrianglesRef()[closest.index].getNormal()));
					photon.beam.origin = closest.position + photon.beam.direction * glm::vec3(0.000001);
				}*/
			}
		}
		printf("Gathered %d photons of data.\n", gathered_photons.size());
	}

	PhotonMap::PhotonMap(PhotonMapper* _p) : p(*_p), photon_map(3, (*_p), nanoflann::KDTreeSingleIndexAdaptorParams(10))
	{
		//photon_map(3, points, nanoflann::KDTreeSingleIndexAdaptorParams(10)), p(points)
		//photon_tree_t photon_map(3, p, nanoflann::KDTreeSingleIndexAdaptorParams(10));
		photon_map.buildIndex();
		printf("Size of photon_map = %d", photon_map.size());
	}

	//Finds the N nearest neighbours to a given point and returns the indices + the squared euclidean distance to each
	void PhotonMap::findNNearestPoints(glm::vec3 point, std::vector<size_t>& ret_index, std::vector<float>& out_dist_sqr, unsigned int number_of_results) {
		//std::vector<size_t>   ret_index(number_of_results);
		//std::vector<float>    out_dist_sqr(number_of_results);

		float query_pt[3] = { point.x, point.y, point.z };
		number_of_results = photon_map.knnSearch(&query_pt[0], number_of_results, &ret_index[0], &out_dist_sqr[0]);

		// In case of less points in the tree than requested:
		ret_index.resize(number_of_results);
		out_dist_sqr.resize(number_of_results);
	}

	glm::vec3 PhotonMap::gatherPhotons(glm::vec3 point, glm::vec3 normal) {
		std::vector<size_t> nearest_points(50);
		std::vector<float> sqr_distances(50);
        const float sqr_radius = 0.1;

		findNNearestPoints(point, nearest_points, sqr_distances, 50);
		
		glm::vec3 energy(0.0, 0.0, 0.0);
		for (int i = 0; i < nearest_points.size(); i++) {
			if (sqr_distances[i] > sqr_radius) continue;
            float dot = -glm::dot(normal, p.getDirection(nearest_points[i]));
            if(dot < 0.0f) continue;
            
			float weight = (3.0 / M_PI * std::sqrt(1.0f - (sqr_distances[i] / sqr_radius))) * dot; //Single photon diffuse lighting
            //double weight = simpsonKernel(neighbour.distance / sqRadius) * dot; return 3.0 / PI * sqr(1 - sqx);
			//weight *= (1.0 - std::sqrt(sqr_distances[i])) / 500.0;
			energy += p.getEnergy(nearest_points[i]) * glm::vec3(weight);
		}
		//printf("Energy = (%f, %f, %f); %d\n", energy.x, energy.y, energy.z, nearest_points.size());
		return glm::clamp(energy * glm::vec3(3 / M_PI / 0.1), glm::vec3(0.0), glm::vec3(1.0));
	}
}
