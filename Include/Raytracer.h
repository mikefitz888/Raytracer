#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <glm/glm.hpp>
#include <vector>
#include "ModelLoader.h"

class Triangle;
struct Intersection;

using model::Scene;
using model::AABB;

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;

	Ray(glm::vec3 o, glm::vec3 d) : origin(o), direction(d) {}
	
	inline glm::vec3 getPoint(float t) { 
		return origin + direction * glm::vec3(t);  
	} // t > 0

	glm::vec3 getIntersection(Triangle& triangle);
	bool closestIntersection(/*const std::vector<Triangle>& triangles*/const Scene &scene, Intersection& closestIntersection);
    bool ray_aabb_intersect(glm::vec3 origin, glm::vec3 inverseDirection, const AABB box);

};

class Trace {

};

#endif // !RAYTRACER_H

