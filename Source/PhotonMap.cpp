#include "../Include/PhotonMap.h"

namespace photonmap {
	Photon::Photon(glm::vec3 o, glm::vec3 d, unsigned int depth) : beam(o, d) { }

	void PhotonMapper::mapScene(model::Scene& scene) {
		float total_light_intensity = 0.0f;
		for (auto light : scene.light_sources) { total_light_intensity += light->intensity; }

		unsigned int assigned_photons = 0;
		std::vector<unsigned int> photons_per_light;
		for (int i = 0; i < scene.light_sources.size(); i++) {
			photons_per_light[i] = (unsigned int)(((float)number_of_photons) * (scene.light_sources[i]->intensity / total_light_intensity));
			assigned_photons += photons_per_light[i];
		}

		photons_per_light[scene.light_sources.size() - 1] += number_of_photons - assigned_photons; //Give any unused photons (due to truncation) to the last light source
		
		/*
		** At this point the photons have been divided between available light sources, the split being weighted by the intensity of the light source
		** (although I am not sure if this is the best approach. It shouldn't matter for simple examples however.)
		**
		** First step is to generate random directions for each photon based on an appropriate probability density
		** Next step is to simulate the action of the photon throughout the scene, employing russion-roulette approach for randomising photon action
		**   -
		*/

		//Generate photons
		for (int i = 0; i < scene.light_sources.size(); i++) {
			while (photons_per_light[i]--) {
				//Light uniformly distributed in all directions, not necessarily the best approach, we probably want to stop upwards directed light
				photons.emplace_back(scene.light_sources[i]->position, glm::sphericalRand(1.0f), number_of_bounces);
			}
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
				//Store photon information (color/energy, position, direction)
				gathered_photons.emplace_back(closest.color * glm::vec3(1 / sqrt(number_of_bounces-photon.depth)) , closest.position, photon.beam.direction);

				//Calculate shadow photon, essentially by finding 2nd closest intersection. Bumping origin of ray slightly to prevent trivial collision
				//These may need to be stored separately, will find out after testing
				photon.beam.origin = closest.position + photon.beam.direction * glm::vec3(0.000001);
				if (photon.beam.closestIntersection(scene.getTrianglesRef(), shadow)) {
					gathered_photons.emplace_back(glm::vec3(-0.25f), shadow.position, photon.beam.direction);
				}

				//Bounce photon
				photon.beam.direction = glm::normalize(glm::reflect(photon.beam.direction, scene.getTrianglesRef()[closest.index].getNormal()));
				photon.beam.origin = closest.position + photon.beam.direction * glm::vec3(0.000001);
			}
		}
	}
}