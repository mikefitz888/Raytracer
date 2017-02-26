#include "../Include/PhotonMap.h"

//#include "../Include/SDLauxiliary.h"
#define _GLOBAL_ILLUMINATION_ENABLE_ true
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

#if _GLOBAL_ILLUMINATION_ENABLE_==1
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

        

#pragma omp parallel
        {
            Intersection closest, shade;
            std::vector<PhotonInfo> thread_indirect_photons, thread_direct_photons, thread_shadow_photons;
#pragma omp for schedule(dynamic, 200)
            for (int i = 0; i < number_of_photons; i++) {
                if (i % (number_of_photons / 10) == 0) {
                    std::cout << "     " << i << " of " << number_of_photons << " generated" << std::endl;
                }
                glm::vec3 origin = glm::linearRand(glm::vec3(-0.35, -0.80f, -0.35), glm::vec3(0.35, -0.95f, 0.35));
                float u = Rand(); float v = 2 * M_PI * Rand();
                //glm::vec3 direction = MATH::CosineWeightedHemisphereDirection(scene.light_sources[0]->direction); // <-- seems to cause weird floor pattern?
                glm::vec3 direction = glm::linearRand(glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                Ray ray(origin, direction);
                glm::vec3 radiance = glm::dot(ray.direction, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec3(1.0f); //Last term is light color (white), weighted by light direction
                //glm::vec3 radiance = glm::vec3(1.0f);

                for (int bounce = 0; bounce < number_of_bounces; bounce++) {

                    //bounce == 0 => direct lighting => shoot shadow photons
                    if (ray.closestIntersection(scene, closest)) {



                        // determine intersection data
                        glm::vec3 intersection = closest.position;
                        glm::vec3 normal = closest.triangle->getNormal();
                        //glm::vec3 reflection = MATH::CosineWeightedHemisphereDirection(normal);;
                        glm::vec3 reflection = normal;
                        reflection = glm::rotateX(reflection, glm::linearRand(-1.0f, 1.0f) * (float)PI / 2.0f);
                        reflection = glm::rotateY(reflection, glm::linearRand(-1.0f, 1.0f) * (float)PI / 2.0f);
                        reflection = glm::rotateZ(reflection, glm::linearRand(-1.0f, 1.0f) * (float)PI / 2.0f);
                        //glm::vec3 reflection = glm::linearRand(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                        reflection = glm::normalize(reflection);

                        radiance *= glm::dot(normal, reflection);
                        radiance = glm::clamp(radiance, 0.0f, 1.0f);

                        // Weaken ray intensity based on distance travelled
                        float distance = glm::length(intersection - ray.origin);
                        float distance_factor = 1.0f - glm::clamp(distance / 4.5f, 0.0f, 1.0f);
                        radiance *= distance_factor;

                        // Perform different operations on indirect light and direct light
                        if (bounce > 0) { //Indirect lighting


                            thread_indirect_photons.emplace_back(radiance, intersection, ray.direction);

                            float p = (radiance.r + radiance.g + radiance.b) * 0.9;
                            float rand = (float)std::rand() / (float)RAND_MAX;
                            if (rand > p) {
                                break;
                            }
                        } else { //Direct lighting
                            thread_direct_photons.emplace_back(radiance, intersection, ray.direction);

                            //Add shadows
                            Ray shadow(intersection + 0.001f * ray.direction, ray.direction);
                            while (shadow.closestIntersection(scene, shade)) {
                                thread_shadow_photons.emplace_back(glm::vec3(0), shade.position, ray.direction);
                                shadow.origin = shade.position + 0.001f * ray.direction;
                            }
                        }

                        // Weighted control to make a number of photons not transfer colour
                        float rand_color = (float)rand() / (float)RAND_MAX;
                        if (rand_color > 0.95) {
                            radiance = glm::max(0.0f, glm::dot(glm::normalize(-ray.direction), glm::normalize(normal))) * (radiance);
                        } else {
                            radiance = glm::max(0.0f, glm::dot(glm::normalize(-ray.direction), glm::normalize(normal))) * (radiance * closest.triangle->color);
                        }

                        ray.origin = intersection + 0.001f*normal;
                        ray.direction = reflection;
                    } else break;
                }
            }
        #pragma omp critical 
            {
                for (auto& p : thread_direct_photons) {
                    direct_photons.emplace_back(p.color, p.position, p.direction);
                }
                for (auto& p : thread_direct_photons) {
                    indirect_photons.emplace_back(p.color, p.position, p.direction);
                }
                for (auto& p : thread_shadow_photons) {
                    shadow_photons.emplace_back(p.color, p.position, p.direction);
                }
            }
        }
		//Weight photons
		for (auto photon : direct_photons.photons) {
			photon.color /= (direct_photons.size() + indirect_photons.size());
		}
		for (auto photon : indirect_photons.photons) {
			photon.color /= (direct_photons.size() + indirect_photons.size());
		}
#endif

        // ******************************************************************************** //
        // CAUSTIC STEP
        /*
            Thoughts:
            - First hit photons ignored, must have passed through a refractive medium first?
        
        */

        // Load from cache opportunity
        bool load_caustic_map = false;
        if (caustic_photons.file_exists("caustic_data.ptm")) {
            std::cout << std::endl;
            std::cout << "Caustic map data exists in 'caustic_data.ptm' would you like to load this? (Type 'y' for Yes)" << std::endl;
            char c;
            std::cin >> c;

            if (c == 'y' || c == 'Y') {
                load_caustic_map = true;
            }
            std::cout << std::endl;
        }

        if (load_caustic_map) {
            caustic_photons.load("caustic_data.ptm");
        } else {

            // Generate fresh
            std::cout << "GENERATING CAUSTIC MAP: " << std::endl;
            auto& triangles = scene.getTrianglesRef();
            number_of_photons *= 250;
            //number_of_photons /= 100;
#pragma omp parallel
            {
                Intersection closest, shade;
                std::vector<PhotonInfo> thread_photons;
#pragma omp for schedule(dynamic, 500)
                for (int i = 0; i < number_of_photons; i++) {

                    if (i % (number_of_photons / 10) == 0) {
                        std::cout << "     " << i << " of " << number_of_photons << " generated" << std::endl;
                    }

                    glm::vec3 origin = glm::linearRand(glm::vec3(-0.00, -0.75f, -0.00), glm::vec3(0.00, -0.75f, 0.00));
                    float u = Rand(); float v = 2 * M_PI * Rand();
                    glm::vec3 direction = MATH::CosineWeightedHemisphereDirection(scene.light_sources[0]->direction); // <-- seems to cause weird floor pattern?
                    //glm::vec3 direction = glm::linearRand(glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                    Ray ray(origin, direction);

                    // Colour
                    glm::vec3 radiance = glm::vec3(1.0f);


                    // We only want to allow photons to refract a number of times
                    bool hasRefracted = false;

                    const int RAY_DEPTH_MAX = 20;

                    for (int ray_depth = 0; ray_depth < RAY_DEPTH_MAX; ray_depth++) {


                        if (ray.closestIntersection(scene, closest)) {
                            //Triangle triangle  = scene.getTrianglesRef()[closest.index];
                            Material *material = closest.triangle->getMaterial();

                            // Calculate triangle surface normal (using barycentric coordinates)
                            glm::vec3 barycentric_coords = closest.triangle->calculateBarycentricCoordinates(closest.position);
                            glm::vec3 surface_normal = (closest.triangle->n0*barycentric_coords.x + closest.triangle->n1*barycentric_coords.y + closest.triangle->n2*barycentric_coords.z);

                            if (closest.triangle->hasMaterial() && material->getType() != MaterialType::NORMAL) {


                                switch (material->getType()) {

                                    // REFRACTIVE
                                    case MaterialType::REFRACTIVE:
                                    {

                                        // DETERMINE NEW REFRACTED RAY
                                        float refractive_index_air = 1.00003f;
                                        float refractive_index_material = material->getRefractiveIndex();
                                        float eta = 1.0;

                                        glm::vec3 refr_dir;
                                        if (glm::dot(glm::normalize(surface_normal), glm::normalize(direction)) < 0.0) {
                                            eta = refractive_index_air / refractive_index_material;
                                            refr_dir = glm::normalize(glm::refract(glm::normalize(direction), glm::normalize(surface_normal), eta));
                                        } else {
                                            eta = refractive_index_material / refractive_index_air;
                                            refr_dir = -glm::normalize(glm::refract(glm::normalize(-direction), glm::normalize(surface_normal), eta));
                                        }

                                        glm::vec3 refr_ray_pos = closest.position + refr_dir*0.0001f;

                                        // Set next ray properties
                                        ray.origin = refr_ray_pos;
                                        ray.direction = refr_dir;

                                        // Adjust colour
                                        radiance *= closest.triangle->color;

                                        hasRefracted = true;
                                        continue;
                                    } break;

                                    // REFLECTIVE
                                    case MaterialType::REFLECTIVE:
                                    {

                                    } break;
                                }
                            } else {
                                // HITS A NORMAL DIFFUSE SURFACE (ABSORB if refracted before)
                                if (hasRefracted) {

                                    thread_photons.emplace_back(radiance, closest.position, ray.direction);
                                    //caustic_photons.emplace_back(radiance, closest.position, ray.direction);
                                    //ray_depth = RAY_DEPTH_MAX; // <- stop tracing
                                    //hasRefracted = false;
                                } /*else {*/
                                    // Reflect off of diffuse surface
                                    /*glm::vec3 reflection = surface_normal;
                                    reflection = glm::rotateX(reflection, glm::linearRand(-1.0f, 1.0f) * (float)PI / 2.0f);
                                    reflection = glm::rotateY(reflection, glm::linearRand(-1.0f, 1.0f) * (float)PI / 2.0f);
                                    reflection = glm::rotateZ(reflection, glm::linearRand(-1.0f, 1.0f) * (float)PI / 2.0f);
                                    //glm::vec3 reflection = glm::linearRand(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
                                    reflection = glm::normalize(reflection);

                                    // Set next ray properties
                                    ray.origin = closest.position + reflection*0.0001f;
                                    ray.direction = reflection;*/
                                    //}
                                ray_depth = RAY_DEPTH_MAX; // <- stop tracing
                                continue;
                            }
                        }
                    }

                }

#pragma omp critical
                {
                    for (auto& p : thread_photons) {
                        caustic_photons.emplace_back(p.color, p.position, p.direction);
                    }
                }
            }

            // Collect

            // Weight
            for (auto& photon : caustic_photons.photons) {
                photon.color /= (caustic_photons.size());
                photon.color *= 100000.0f;
            }

        }

		/*for (int i = 0; i < number_of_photons; i++) {
			glm::vec3 intensity(std::pow(2.0f, scene.light_sources[0]->intensity)/photons_per_light[0]);
			glm::vec3 origin = scene.light_sources[0]->position;//glm::vec3(-0.1, -0.85f, -0.1);//glm::linearRand(glm::vec3(-0.1, -0.85f, -0.1), glm::vec3(0.1, -0.99f, 0.1));

			float u = Rand();
			float v = 2 * M_PI * Rand();
			glm::vec3 direction = glm::vec3(std::cos(v)*std::sqrt(u), std::sin(v)*std::sqrt(u), std::sqrt(1-u));

			Ray ray(origin, direction); //Ray is not being updated each iteration
			unsigned int bounce_limit = number_of_bounces;
			while (bounce_limit-- && ray.closestIntersection(scene.getTrianglesRef(), closest)) {
                //bounce_limit = 0;
				glm::vec3 pd = closest.color;// glm::vec3(3); //Pr(Diffuse reflection)
				glm::vec3 ps(0.0); //Pr(Specular reflection)

				float Pr = std::max(pd.r + ps.r, std::max(pd.g + ps.g, pd.b + ps.b));
				float Pd = Pr * (pd.r + pd.g + pd.b) / (pd.r + pd.g + pd.b + ps.r + ps.g + ps.b);
				float Ps = Pr - Pd;

				float r = Rand();
				if (r < Pd) { //Diffuse reflection
					gathered_photons.emplace_back(intensity, closest.position, ray.direction); //Store photon
					ray.direction = glm::normalize(glm::reflect(ray.direction, scene.getTrianglesRef()[closest.index].getNormal())); //Reflect photon
					ray.origin = closest.position + ray.direction * glm::vec3(0.000001); //Slight offset to prevent colliding with current geometry
					intensity *= pd / Pd; //Adjust powers to suit probability of survival
				}
				else if (r < Pd + Ps) { //Specular reflection
					intensity *= ps / Ps;
				}
				else { //Absorb photon (TODO: only store if diffuse surface)
					gathered_photons.emplace_back(intensity, closest.position, ray.direction); //Store photon
					break;
				}
			}
            if(i % 100 == 0){printf("Photon = %d\n", i);}
		}*/
		//printf("Gathered %d photons of data.\n", gathered_photons.size());
		printf("Gathered %d photons of direct data.\n", direct_photons.size());
		printf("Gathered %d photons of indirect data.\n", indirect_photons.size());
		printf("Gathered %d photons of shadow data.\n", shadow_photons.size());
        printf("Gathered %d photons of caustic data.\n", caustic_photons.size());

        // Save caustic data data:
        bool save_photon_data = false;
        if (!load_caustic_map) {
            std::cout << std::endl;
            std::cout << "Would you like to save the caustic data?" << std::endl;
            char c;
            std::cin >> c;
            std::cout << std::endl;
            if (c == 'y' || c == 'Y') {
                caustic_photons.save("caustic_data.ptm");
            }
        }
        
	}

	PhotonMap::PhotonMap(PhotonMapper* _p) : p(*_p), 
		direct_photon_map(3, (_p->direct_photons), nanoflann::KDTreeSingleIndexAdaptorParams(10)),
		indirect_photon_map(3, (_p->indirect_photons), nanoflann::KDTreeSingleIndexAdaptorParams(10)),
		shadow_photon_map(3, (_p->shadow_photons), nanoflann::KDTreeSingleIndexAdaptorParams(10)),
        caustic_photon_map(3, (_p->caustic_photons), nanoflann::KDTreeSingleIndexAdaptorParams(10))
	{
		//photon_map(3, points, nanoflann::KDTreeSingleIndexAdaptorParams(10)), p(points)
		//photon_tree_t photon_map(3, p, nanoflann::KDTreeSingleIndexAdaptorParams(10));
		direct_photon_map.buildIndex();
		indirect_photon_map.buildIndex();
		shadow_photon_map.buildIndex();
        caustic_photon_map.buildIndex();
		//printf("Size of photon_map = %d", photon_map.size());
	}

	void PhotonMap::getDirectPhotonsRadius(const glm::vec3& pos, const float radius, std::vector< std::pair<size_t, float> >& indices) {
		const float point[3] = { pos.x, pos.y, pos.z };
		nanoflann::SearchParams params;
		params.sorted = false;
		const size_t count = direct_photon_map.radiusSearch(point, radius, indices, params);
	}

	void PhotonMap::getIndirectPhotonsRadius(const glm::vec3& pos, const float radius, std::vector< std::pair<size_t, float> >& indices) {
		const float point[3] = { pos.x, pos.y, pos.z };
		nanoflann::SearchParams params;
		params.sorted = false;
		const size_t count = indirect_photon_map.radiusSearch(point, radius, indices, params);
	}

	void PhotonMap::getShadowPhotonsRadius(const glm::vec3& pos, const float radius, std::vector< std::pair<size_t, float> >& indices) {
		const float point[3] = { pos.x, pos.y, pos.z };
		nanoflann::SearchParams params;
		params.sorted = false;
		const size_t count = shadow_photon_map.radiusSearch(point, radius, indices, params);
	}

    void PhotonMap::getCausticPhotonsRadius(const glm::vec3& pos, const float radius, std::vector< std::pair<size_t, float> >& indices) {
        const float point[3] = { pos.x, pos.y, pos.z };
        nanoflann::SearchParams params;
        params.sorted = false;
        const size_t count = caustic_photon_map.radiusSearch(point, radius, indices, params);
    }

	//void PhotonMap::getCausticPhotonsRadius() {}

	bool PhotonMap::nearestDirectPhotonInRange(const glm::vec3& pos, const float radius, size_t& index) {}

}
