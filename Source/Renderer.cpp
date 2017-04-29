#include "../Include/Renderer.h"

namespace RENDERER {
	vec3 deepGather(vec3 point, vec3 normal, model::Scene& scene, PhotonMap& photon_map, PhotonMapper& photon_mapper, size_t depth, size_t secondary_rays) {
		if (depth == 0) {
			//return photon map estimate
			std::vector<std::pair<size_t, float>> direct_photons_in_range, shadow_photons_in_range, indirect_photons_in_range;
			photon_map.getIndirectPhotonsRadius(point, 0.01, indirect_photons_in_range);

			glm::vec3 total_colour_energy, total_light_energy, total_energy;
			float samples_intensity = 0;
			float samples_colour_bleed = 0;

			for (auto& pht : indirect_photons_in_range) {
				size_t id = pht.first;
				glm::vec3 energy = photon_mapper.indirect_photons.photons[id].color;
				float distance = pht.second;

				// Check that sample direction matches surface normal
				//glm::vec3 normal = normal;
				glm::vec3 photon_direction = photon_mapper.indirect_photons.photons[id].direction;
				float     normal_factor = 1.0 - glm::dot(photon_direction, normal);
				if (normal_factor >= 0) {
					//float distance_factor = 1.0; // If using sampling method 1), disable this
					float distance_factor = 1.0f - glm::clamp(distance / 0.1, 0.0, 1.0);

					samples_colour_bleed += normal_factor*glm::length(energy); //<-- gives a very smooth result, but technically not all that correct (example: http://i.imgur.com/g8EIXIJ.png)
					samples_intensity += normal_factor; // <-- Only weight samples by their contribution. This reduces visual artifacts
					total_colour_energy += energy*normal_factor*distance_factor;//*(1.0f  - (float)glm::sqrt(distance)/ (float)sqrt(PHOTON_GATHER_RANGE));
					total_light_energy += glm::vec3(glm::length(energy))*distance_factor;
				}

			}
			total_colour_energy /= samples_colour_bleed;
			//total_colour_energy /= 100;//samples_colour_bleed;
			total_light_energy /= 250;
			total_energy = ((total_light_energy + 1.05f) * total_colour_energy)/*0.5f*/;
			//total_energy *= 0.5;

			//photon_radiance = closest_intersect.color*total_energy;
			return total_energy;
		}

		vec3 radiance(0.0f);
		Intersection random;

		int samples = 0;
		for (int i = 0; i < secondary_rays; i++) {
			glm::vec3 ray_dir = MATH::CosineWeightedHemisphereDirection(normal);
			Ray ray(point+ ray_dir*0.001f, ray_dir);
			if (ray.closestIntersection(scene, random)) {
				vec3 weight = dot(ray.direction, normal) * random.triangle->color; //Experimental color bleeding
				radiance += deepGather(random.position, random.triangle->getNormal(), scene, photon_map, photon_mapper, depth-1, secondary_rays) * weight;
				samples++;
			}
		}
		return radiance / (float)samples;
		//return radiance /= secondary_rays;
	}

	vec3 finalGather(vec3 point, Triangle* triangle, model::Scene& scene, PhotonMap& photon_map, PhotonMapper& photon_mapper, size_t depth, size_t secondary_rays) {
		Intersection random;
		vec3 radiance(0.0f);
		int samples = 0;
		for (int i = 0; i < secondary_rays; i++) {
			vec3 normal = triangle->getNormal();
			glm::vec3 ray_dir = MATH::CosineWeightedHemisphereDirection(normal);
			Ray ray(point+ ray_dir*0.001f, ray_dir);
			if (ray.closestIntersection(scene, random)) {
				vec3 weight = dot(ray.direction, normal) * random.triangle->color;
				radiance += deepGather(random.position, random.triangle->getNormal(), scene, photon_map, photon_mapper, depth - 1, secondary_rays) * weight;
				samples++;
			}
		}
		radiance /= (float)samples;
		//radiance /= secondary_rays;

		//Optional: apply BRDF to this value before returning. This needs to be done by the calling function.
		return radiance;
	}
}