#include "../Include/Raytracer.h"
#include "../Include/Triangle.h"


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

bool Ray::closestIntersection(/*const std::vector<Triangle>& triangles*/const Scene &scene, Intersection& closestIntersection) {
	bool result = false;

	float min_dist = std::numeric_limits<float>::max();
	glm::vec3 position;
    glm::vec3 inverse_direction = 1.0f/this->direction;
	int index;

	//int i = -1;
    Triangle *triangle_result = nullptr;
    for (auto* model : scene.models) {

        // Check for collision with model outer bounding box:
        //if (!model->use_optimising_structure || ray_aabb_intersect(this->origin, inverse_direction, model->getBoundingBox())) {

            
            std::vector<std::vector<Triangle>*> triangle_arrays;
            if (model->getUseOptimisationStructure()) {
                model->getOctree()->getIntersectingSections(origin, inverse_direction, triangle_arrays);
            } else {
                triangle_arrays.push_back(model->getFaces());
            }


            // LOOP THROUGH TRIANGLES IN TRIANGLE ARRAY
            for( std::vector<Triangle>* triangles : triangle_arrays){
                for (auto& triangle : *triangles) {
                    //i++;
                    glm::mat3 A(-direction, triangle.e1(), triangle.e2());
                    glm::vec3 b = origin - triangle.v0;

                    float detA = glm::determinant(A); //Calculating determinant of A for use in Cramer's rule

                    glm::mat3 A0 = A;
                    A0[0] = b;
                    float detA0 = glm::determinant(A0);
                    float t = detA0 / detA;

                    if (t >= 0) { //If t is negative then no intersection will occur
                        //If this is a bottle-neck then maybe it's faster to: glm::inverse(A) * b;
                        glm::mat3 A1 = A;
                        A1[1] = b;
                        float detA1 = glm::determinant(A1);
                        float u = detA1 / detA;
                        if (u < 0 || u > 1) continue;

                        glm::mat3 A2 = A;
                        A2[2] = b;
                        float detA2 = glm::determinant(A2);
                        float v = detA2 / detA;
                        if (v < 0 || v > 1) continue;



                        //Check if intersection with triangle actually occurs
                        if (u + v <= 1) {
                            glm::vec3 intersect = triangle.v0 + u*triangle.e1() + v*triangle.e2();
                            float distance = glm::distance(origin, intersect);
                            if (distance < min_dist) {

                                // Check normal
                                float factor = glm::dot(glm::normalize(direction), glm::normalize(triangle.normal));
                                if (factor > 0.001 && !triangle.twoSided) { continue; }

                                result = true;
                                min_dist = distance;
                                position = intersect;
                                //index = i;
                                triangle_result = &triangle;
                            }
                        }
                    }
                }
            //}
        }
    }
	if (result) {
		closestIntersection.distance = min_dist;
		closestIntersection.position = position;
		//closestIntersection.index = index;
        closestIntersection.triangle = triangle_result;//triangles[index];
	}
	return result;
}

bool Ray::ray_aabb_intersect(glm::vec3 origin, glm::vec3 inverseDirection, const AABB box) {

    double tx1 = (box.min.x - origin.x)*inverseDirection.x;
    double tx2 = (box.max.x - origin.x)*inverseDirection.x;

    double tmin = glm::min(tx1, tx2);
    double tmax = glm::max(tx1, tx2);

    double ty1 = (box.min.y - origin.y)*inverseDirection.y;
    double ty2 = (box.max.y - origin.y)*inverseDirection.y;

    tmin = glm::max(tmin, glm::min(ty1, ty2));
    tmax = glm::min(tmax, glm::max(ty1, ty2));

    return tmax >= tmin && tmax >= 0;
}