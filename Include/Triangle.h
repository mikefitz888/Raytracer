#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <glm/glm.hpp>
#include <vector>
#include "bitmap_image.hpp"

// Used to describe a triangular surface:
enum MaterialType { NORMAL, REFLECTIVE, REFRACTIVE };
/*
    - Material types are determined from type rather than using inheritance as it would
    be expensive to define a per-material function. Instead, the properties will be used
    as modifiers in the render process.

    NORMAL:
        - Standard diffuse lighting with optional specular highlight

    REFLECTIVE:
        - extension of normal with optional control of glossiness 

    REFRACTIVE:
        - Light rays pass through the object. 

*/

class Material {
    bitmap_image* diffuse_map, specular_map, normal_map;

    MaterialType type = MaterialType::NORMAL;
    float specular_factor   = 0.0f;  // between 0.0 and 1.0 (intensity of specular highlight)
    float specular_power    = 32.0f; // Specular power factor (controls highlight size)
    float reflection_factor = 0.0f;  // between 0.0 and 1.0 (factor of reflectivity vs diffuse)
    float glossiness        = 0.0f;  // between 0.0 and 1.0 (controls the glossiness of a surface, 0.0 = pure reflections, 1.0 = matte reflections)
    float refractive_index  = 1.0f;  // Refractive index of the material

public:
    //Material(MaterialType type, float specular_factor = 0.0f, float reflection_factor = 0.0f, float glossiness = 0.0f, float refractive_index = 0.0f);
    inline void materialSetType(MaterialType type){
        this->type = type;
    }
    inline void materialSetTypeNormal() {
        this->type = MaterialType::NORMAL;
    }
    inline void materialSetTypeReflective(float reflection_factor, float glossiness) {
        this->type = MaterialType::REFLECTIVE;
        this->reflection_factor = reflection_factor;
        this->glossiness = glossiness;
    }
    inline void materialSetTypeRefractive(float refractive_index) {
        this->type = MaterialType::REFRACTIVE;
        this->refractive_index = refractive_index;
    }
    
    inline float getSpecularFactor() { return this->specular_factor;  }
    inline float getSpecularPower() { return this->specular_power; }
    inline float getReflectionFactor() { return this->reflection_factor;  }
    inline float getGlossiness() { return this->glossiness; }
    inline float getRefractiveIndex() { return this->refractive_index; }
    inline MaterialType getType() { return this->type;  }

    inline void setSpecularFactor(float specular_factor) { this->specular_factor = specular_factor; }
    inline void setSpecularPower(float specular_power) { this->specular_power = specular_power; }
    inline void setReflectionFactor(float reflection_factor) { this->reflection_factor = reflection_factor; }
    inline void setGlossiness(float glossiness) { this->glossiness = glossiness; }
    inline void setRefractiveIndex(float refractive_index) { this->refractive_index = refractive_index; }
};

class Triangle {
public:
	//Vertices
	glm::vec3 v0, v1, v2;

	//Texture Coordinates
	glm::vec2 uv0, uv1, uv2;

	//Normals
	glm::vec3 n0, n1, n2;
    glm::vec3 edge_0, edge_1;

	//Optional Parameters
    Material *material = nullptr;

    // ID
    int triangle_scene_id = 0;

	glm::vec3 normal;
	glm::vec3 color = glm::vec3(1.0f,1.0f,1.0f);
	bool twoSided = true;

    Triangle& operator=(const Triangle& t) {
        Triangle* con = (new Triangle(v0, v1, v2, uv0, uv1, uv2, n0, n1, n2));
        con->color = color;
        con->material = material;
        con->twoSided = twoSided;
        return *con;
    }

	inline Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2,
					glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2,
					glm::vec3 n0, glm::vec3 n1, glm::vec3 n2) :
					v0(v0), v1(v1), v2(v2), uv0(uv0), uv1(uv1), uv2(uv2), n0(n0), n1(n1), n2(n2), edge_0(v1-v0), edge_1(v2-v0) {
		ComputeNormal();
	}

	Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color, glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2)
		: v0(v0), v1(v1), v2(v2), color(color), uv0(uv0), uv1(uv1), uv2(uv2), edge_0(v1 - v0), edge_1(v2 - v0) {
		ComputeNormal();
	}

	inline const glm::vec3& e1() const { return edge_0; }
	inline const glm::vec3& e2() const { return edge_1; }
    inline void setMaterial(Material* material) { this->material = material; }
    inline Material* getMaterial() { return this->material; }
    inline bool hasMaterial() { return this->material != nullptr; }

	inline bool checkIntersection(glm::vec3 intersect) {
		return (0 < intersect.y) && (0 < intersect.z) && (intersect.y + intersect.z < 1);
	}

	inline glm::vec3 getNormal() {
		return normal;
	}

	inline void ComputeNormal() {
        //Not sure why, but if these aren't here (i.e. the initializer list sets them) then it breaks
        edge_0 = v1 - v0;
        edge_1 = v2 - v0;
		/*
		**  Points in triangle's plane: r = v0 + u*e1 + v*e2
		**	Points in triangle: 0 < u, 0 < v, u+v < 1
		*/
		normal = glm::normalize(glm::cross(edge_1, edge_0));
	}

    inline void ComputeNormalPerVertex() {
        ComputeNormal();
        n0 = normal;
        n1 = normal;
        n2 = normal;
    }

	/*
		Fills the internal triangle data with the barycentric coordinates for a given point of intersection.
		These can then be used for sampling interpolated uv coordinates/colours at any point on the triangle.

		RETURNS:
			- Vec3 where each factor corresponds to the contribution from each vertex of the triangle (0,1,2).

		Source:
			http://answers.unity3d.com/questions/383804/calculate-uv-coordinates-of-3d-point-on-plane-of-m.html
	*/
	inline glm::vec3 calculateBarycentricCoordinates(glm::vec3 intersection_point) {
		glm::vec3 factorA = v0 - intersection_point;
		glm::vec3 factorB = v1 - intersection_point;
		glm::vec3 factorC = v2 - intersection_point;

		float area = glm::length( glm::cross(v0 - v1, v0 - v2));
		glm::vec3 barycentric_coordinates;
		barycentric_coordinates.x = glm::length(glm::cross(factorB, factorC)) / area;
		barycentric_coordinates.y = glm::length(glm::cross(factorC, factorA)) / area;
		barycentric_coordinates.z = glm::length(glm::cross(factorA, factorB)) / area;
		return barycentric_coordinates;
	}
};

struct Intersection {
	glm::vec3 position;
	//glm::vec3 color;
    Material*  mat;
	float distance;
	//int index;
    Triangle* triangle;
	Intersection(glm::vec3 pos, float dis, int ind, Triangle* t) : position(pos), distance(dis)/*, index(ind)*/,triangle(t) {}
	Intersection(){}
};

#endif // !TRIANGLE_H
