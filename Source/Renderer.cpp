#include "../Include/Renderer.h"

namespace RENDERER {
	vec3 deepGather(vec3 point, vec3 normal, std::vector<Triangle>& triangles, PhotonMap& photon_map, PhotonMapper& photon_mapper, size_t depth, size_t secondary_rays) {
		if (depth == 0) {
			//return photon map estimate
			std::vector<std::pair<size_t, float>> direct_photons_in_range, shadow_photons_in_range, indirect_photons_in_range;
			photon_map.getIndirectPhotonsRadius(point, 0.1, indirect_photons_in_range);

			glm::vec3 total_colour_energy, total_light_energy, total_energy;
			float samples_intensity = 0;

			for (auto pht : indirect_photons_in_range) {
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

					//samples_colour_bleed += normal_factor*glm::length(energy); //<-- gives a very smooth result, but technically not all that correct (example: http://i.imgur.com/g8EIXIJ.png)
					samples_intensity += normal_factor; // <-- Only weight samples by their contribution. This reduces visual artifacts
					//total_colour_energy += energy*normal_factor*distance_factor;//*(1.0f  - (float)glm::sqrt(distance)/ (float)sqrt(PHOTON_GATHER_RANGE));
					total_light_energy += glm::vec3(glm::length(energy))*distance_factor;
				}

			}

			total_light_energy /= 200;
			total_energy = (total_light_energy)/*0.5f*/;

			//photon_radiance = closest_intersect.color*total_energy;
			return total_energy;
		}

		vec3 radiance(0.0f);
		Intersection random;
		for (int i = 0; i < secondary_rays; i++) {
			Ray ray(point, MATH::CosineWeightedHemisphereDirection(normal));
			if (ray.closestIntersection(triangles, random)) {
				vec3 weight = dot(ray.direction, normal) * random.color; //Experimental color bleeding
				radiance += deepGather(random.position, triangles[random.index].getNormal(), triangles, photon_map, photon_mapper, depth-1, secondary_rays) * weight;
			}
		}

		return radiance /= secondary_rays;
	}

	vec3 finalGather(vec3 point, size_t intersecting_triangle_id, std::vector<Triangle>& triangles, PhotonMap& photon_map, PhotonMapper& photon_mapper, size_t depth, size_t secondary_rays) {
		Intersection random;
		vec3 radiance(0.0f);
		for (int i = 0; i < secondary_rays; i++) {
			vec3 normal = triangles[intersecting_triangle_id].getNormal();
			Ray ray(point, MATH::CosineWeightedHemisphereDirection(normal));
			if (ray.closestIntersection(triangles, random)) {
				vec3 weight = dot(ray.direction, normal) * random.color;
				radiance += deepGather(random.position, triangles[random.index].getNormal(), triangles, photon_map, photon_mapper, depth - 1, secondary_rays) * weight;
			}
			radiance /= secondary_rays;
		}

		//Optional: apply BRDF to this value before returning. This needs to be done by the calling function.
		return radiance;
	}
}