#ifndef MODELLOADER_H
#define MODELLOADER_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_integer.hpp>
#include "Triangle.h"
#include "bitmap_image.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

namespace model {
	class Model;

	enum LTYPE : unsigned int {POINT=0};
	struct LightSource {
		float intensity;
		glm::vec3 position;
		glm::vec3 direction;
		LTYPE type;
		inline LightSource(glm::vec3 p, glm::vec3 d, float i = 1.0f, LTYPE t = LTYPE::POINT) : position(p), direction(d), intensity(i), type(t) {}
	};

	// AABB
    struct AABB {
        glm::vec3 min, max;
    };

    // OCTREE
    class Octree {
        static const int MAX_OCTREE_DEPTH = 6;

        int depth = 0;
        int child_count = 0;
        AABB bounding_box;
        Octree* parent;
        Octree* children[8];             // Octree node
        std::vector<Triangle> triangles; // Triangles stored in each segment

    public:
        Octree(AABB bounding_box, Octree* parent);
        void init();
        void fill(const std::vector<Triangle>& triangles);
        inline void addTriangle(Triangle t) { triangles.push_back(t); }
        inline int  getTriangleCount() { return triangles.size(); }
        inline int  getDepth() { return this->depth; }
        inline std::vector<Triangle>* getTriangles() { return &this->triangles; }
        void split();   // Splits the octree into subsequent children. If this section has no triangles, it gets ignored.
        void getIntersectingSections(glm::vec3 ray_orig, glm::vec3 ray_inv_dir, std::vector<std::vector<Triangle>*> &storage);


        static bool triangle_aabb_intersection(Triangle& t, AABB& aabb);
        static bool aabb_aabb_intersection(AABB& a, AABB& b);
        static void project(std::vector<glm::vec3> points, glm::vec3 axis, double &min, double &max);
    };

    // MODEL
	class Model {
		//std::vector<Material> materials;
		//std::map<std::string, unsigned int> material_map;
		//unsigned int active_material;
		bool modified = true;

		std::vector<unsigned int> vertexIndices, textureIndices, normalIndices;
		std::vector<glm::vec3> vertices, vertex_normals;
		std::vector<glm::vec2> vertex_textures; //UVs
		std::vector<Triangle> triangles;
		//std::vector<int> mtl_map;

        // Volume structures
        AABB bounding_box;
        Octree* octree = nullptr;
        bool use_optimising_structure = false;
	private:
		void parseVertex(std::istringstream& vertex);
		void parseVertexNormal(std::istringstream& vertex_normal);
		void parseVertexTexture(std::istringstream& vertex_texture);
		void parseFace(std::istringstream& face);
        void CalculateBoundingVolume();
        
        
        
        
		//void parseUseMaterial(std::istringstream& material);
		//void parseMaterialLib(std::istringstream& lib);
	public:
        

		Model(std::string file_name);
        Model(std::vector<Triangle> triangles);
		inline Model() {};
        bool hasOctree();
        Octree* getOctree();
        void generateOctree();



        inline void setUseOptimisationStructure(bool use) {
            this->use_optimising_structure = use;
        }
        inline bool getUseOptimisationStructure() {
            return this->use_optimising_structure;
        }

		/*inline Material& getActiveMaterial(){
			return materials[active_material];
		}*/

		inline void removeFront() {
			triangles.erase(triangles.begin(), triangles.begin() + 2);
		}

		inline void calculateNormals() {
			for (auto t : triangles) {
				t.ComputeNormal();
			}
		}

		//Convert to Triangles for raytracer
		inline std::vector<Triangle>* getFaces() {
			if (modified) {
				modified = false;
				triangles.clear();
				for (int i = 0; i < vertexIndices.size(); i += 3) {
					triangles.emplace_back(
						vertices[vertexIndices[i]],
						vertices[vertexIndices[i + 1]],
						vertices[vertexIndices[i + 2]],
						vertex_textures[textureIndices[i]],
						vertex_textures[textureIndices[i + 1]],
						vertex_textures[textureIndices[i + 2]],
						vertex_normals[normalIndices[i]],
						vertex_normals[normalIndices[i + 1]],
						vertex_normals[normalIndices[i + 2]]
					);
				}
			}
			return &triangles;
		}

        // Get volume structures
        inline AABB getBoundingBox() { return this->bounding_box; }
	};

	struct Scene {
	private:
		std::vector<Triangle> triangles;
        
	public:

		std::vector<Model*> models;
		std::vector<LightSource*> light_sources;
		void getTriangles(std::vector<Triangle>& triangles);

		inline std::vector<Triangle>& getTrianglesRef() { return triangles; }
		inline void addModel(Model* model) {
			models.emplace_back(model);
			//addTriangles(*model->getFaces());
			/*for (auto& t : *model->getFaces()) {
				t.ComputeNormal();
				t.normal = -t.normal;
				t.color = glm::vec3(1.0, 1.0, 1.0);
				triangles.push_back(t);
			}*/
		}

		inline void addLight(LightSource& light) {
			light_sources.emplace_back(&light);
		}

		inline void removeFront() {
			triangles.erase(triangles.begin(), triangles.begin() + 2);
		}

		inline void addTriangles(std::vector<Triangle>& ts) {
			for (auto t : ts) {
				triangles.push_back(t);
			}
		}
	};
}

#endif // !MODELLOADER_H

