#include "Raytracer/Raytracer.h"
#include "Raytracer/Triangle.h"

glm::vec3 Ray::getIntersection(Triangle triangle) {
	//Intersection = t, u, v
	// v0 + u*e1 + v*e2 = s + t*d
	glm::mat3 A(-direction, triangle.e1(), triangle.e2());
	glm::vec3 b = origin - triangle.v0;
	
	// Ax = b, we want to find x = (t, u, v)
	//  x = (A^-1)b

	return glm::inverse(A) * b;

	//Consider cramer's rule to get closed form solution to improve performance
	float detA = glm::determinant(A);
}

bool Ray::closestIntersection(const std::vector<Triangle>& triangles, Intersection& closestIntersection) {
	float min_dist = std::numeric_limits<float>::max();
	for (auto triangle : triangles) {
		glm::mat3 A(-direction, triangle.e1(), triangle.e2());
		glm::vec3 b = origin - triangle.v0;

		float detA = glm::determinant(A); //Calculating determinant of A for use in Cramer's rule

		glm::mat3 A0 = A;
		A0[0] = b;
		float detA0;
		float t = detA0 / detA;

		if (t >= 0) { //If t is negative then no intersection will occur

		}
	}
}