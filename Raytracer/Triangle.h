#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <glm/glm.hpp>
#include <vector>

// Used to describe a triangular surface:
class Triangle {
public:
	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 normal;
	glm::vec3 color;

	Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color)
		: v0(v0), v1(v1), v2(v2), color(color) {
		ComputeNormal();
	}

	inline glm::vec3 e1() { return v1 - v0; }
	inline glm::vec3 e2() { return v2 - v0; }

	inline bool checkIntersection(glm::vec3 intersect) {
		return (0 < intersect.y) && (0 < intersect.z) && (intersect.y + intersect.z < 1);
	}

	inline void ComputeNormal() {
		glm::vec3 e1 = v1 - v0;
		glm::vec3 e2 = v2 - v0;
		/*
		**  Points in triangle's plane: r = v0 + u*e1 + v*e2
		**	Points in triangle: 0 < u, 0 < v, u+v < 1
		*/
		normal = glm::normalize(glm::cross(e2, e1));
	}
};

struct Intersection {
	glm::vec3 position;
	float distance;
	int index;
	Intersection(glm::vec3 pos, float dis, int ind) : position(pos), distance(dis), index(ind) {}
	Intersection(){}
};

#endif // !TRIANGLE_H