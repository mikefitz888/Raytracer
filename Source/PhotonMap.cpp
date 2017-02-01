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
		/*for (int i = 0; i < scene.light_sources.size(); i++) {
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
		}*/

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
		for (int i = 0; i < number_of_photons; i++) {
			glm::vec3 intensity(std::pow(2.0f, scene.light_sources[0]->intensity)/photons_per_light[0]);
			glm::vec3 origin = glm::linearRand(glm::vec3(-0.1, -0.85f, -0.1), glm::vec3(0.1, -0.99f, 0.1));

			float u = Rand();
			float v = 2 * M_PI * Rand();
			glm::vec3 direction = glm::vec3(std::cos(v)*std::sqrt(u), std::sin(v)*std::sqrt(u), std::sqrt(1-u));

			Ray ray(origin, direction);
			unsigned int bounce_limit = 10;
			while (bounce_limit-- && ray.closestIntersection(scene.getTrianglesRef(), closest)) {
				glm::vec3 pd = closest.color; //Pr(Diffuse reflection)
				glm::vec3 ps(0.0); //Pr(Specular reflection)

				float Pr = std::max(pd.r + ps.r, std::max(pd.g + ps.g, pd.b + ps.b));
				float Pd = (pd.r + pd.g + pd.b) / (pd.r + pd.g + pd.b + ps.r + ps.g + ps.b);
				float Ps = (ps.r + ps.g + ps.b) / (pd.r + pd.g + pd.b + ps.r + ps.g + ps.b);

				float r = Rand();
				if (r < Pd) { //Diffuse reflection
					gathered_photons.emplace_back(intensity, closest.position, direction); //Store photon
					direction = glm::normalize(glm::reflect(direction, scene.getTrianglesRef()[closest.index].getNormal())); //Reflect photon
					origin = closest.position + direction * glm::vec3(0.000001); //Slight offset to prevent colliding with current geometry
					intensity *= pd / Pd; //Adjust powers to suit probability of survival
				}
				else if (r < Pd + Ps) { //Specular reflection
					intensity *= ps / Ps;
				}
				else { //Absorb photon
					gathered_photons.emplace_back(intensity, closest.position, direction); //Store photon
					break;
				}
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
