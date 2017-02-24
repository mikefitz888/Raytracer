#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include "PhotonMap.h"

namespace RENDERER {
	using namespace glm;
	using namespace photonmap;

	//Use photon_mapper to look-up kd-tree, or photon_map to retrieve photon by index
	vec3 deepGather(vec3 point, vec3 normal, model::Scene& triangles, PhotonMap& photon_map, PhotonMapper& photon_mapper, size_t depth, size_t secondary_rays);
	vec3 finalGather(vec3 point, Triangle* triangle, model::Scene& triangles, PhotonMap& photon_map, PhotonMapper& photon_mapper, size_t depth = 1, size_t secondary_rays = 10);
	vec3 getRadianceEstimate();
}

#endif // !RENDERER_H
