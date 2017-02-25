#include "../Include/Raytracer.h"
#include "../Include/Triangle.h"


glm::vec3 Ray::getIntersection(Triangle& triangle) {
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

	//float min_dist = std::numeric_limits<float>::max();
	//glm::vec3 position;
    glm::vec3 inverse_direction = 1.0f/this->direction;
	int index;

    closestIntersection.distance = std::numeric_limits<float>::max();

	//int i = -1;
    //Triangle *triangle_result = nullptr;
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
                std::vector<Triangle>& tt = *triangles;
                for (auto& triangle : tt) {

                    glm::vec3 P = glm::cross(direction, triangle.e2());
                    float det = glm::dot(triangle.e1(), P);

                    
                    if (det > -FLT_EPSILON && det < FLT_EPSILON) continue;
                    float inv_det = 1.f / det;

                    glm::vec3 T = origin - triangle.v0;
                    float u = glm::dot(T, P) * inv_det;
                    if (u < 0.f || u > 1.f) continue; //outside triangle

                    glm::vec3 Q = glm::cross(T, triangle.e1());
                    float v = glm::dot(direction, Q) * inv_det;
                    if (v < 0.f || u + v > 1.f) continue; //outside triangle

                    float t = glm::dot(triangle.e2(), Q) * inv_det;
                    if (t > FLT_EPSILON) {
                        glm::vec3 intersect = triangle.v0 + u*triangle.e1() + v*triangle.e2();

                        float dx = origin.x - intersect.x;
                        float dy = origin.y - intersect.y;
                        float dz = origin.z - intersect.z;
                        float distance = dx*dx + dy*dy + dz*dz; //glm::distance(origin, intersect)*glm::distance(origin, intersect); //(intersect.x - origin.x)*(intersect.x - origin.x) + (intersect.y - origin.y)*(intersect.y - origin.y); // glm::distance(origin, intersect);
                        if (distance < closestIntersection.distance) {

                            // Check normal
                            float factor = glm::dot(direction, triangle.normal);
                            if (factor > FLT_EPSILON && !triangle.twoSided) { continue; }

                            result = true;
                            //min_dist = distance;
                            closestIntersection.distance = distance;
                            //position = intersect;
                            closestIntersection.position = intersect;
                            //triangle_result = &triangle;
                            closestIntersection.triangle = &triangle;
                        }
                    }
                }
            //}
        }
    }
	//if (result) {
		closestIntersection.distance = sqrt(closestIntersection.distance);
		//closestIntersection.position = position;
        //closestIntersection.triangle = triangle_result;//triangles[index];
	//}
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