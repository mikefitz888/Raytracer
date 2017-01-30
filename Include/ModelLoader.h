#ifndef MODELLOADER_H
#define MODELLOADER_H
#include <glm/glm.hpp>
#include "Triangle.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>

namespace model {

	class Material {
	public:
		glm::vec3 color;
		inline Material(glm::vec3 c) : color(c) {};
	};

	class Model {
		std::vector<Material> materials;
		std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
		std::vector<glm::vec3> vertices, vertex_normals;
		std::vector<glm::vec2> vertex_textures; //UVs

		std::vector<glm::uvec3> faces;
		std::vector<int> mtl_map;

	private:
		void parseVertex(std::istringstream& vertex);
		void parseVertexNormal(std::istringstream& vertex_normal);
		void parseVertexTexture(std::istringstream& vertex_texture);
		void parseFace(std::istringstream& face);
	public:
		Model(std::string file_name);
		inline Model() {};
		inline void addVertex(glm::vec3 vertex) {
			vertices.push_back(vertex);
		}

		inline void addFace(int a, int b, int c, int color) {
			//faces.push_back(Triangle(vertices[a], vertices[b], vertices[c], color));
			faces.push_back(glm::uvec3(a, b, c));
			mtl_map.push_back(color);
		}

		inline void addMaterial(glm::vec3 color) {
			materials.push_back(Material(color));
		}

		//Convert to Triangles for raytracer
		inline std::vector<Triangle> getFaces() {
			std::vector<Triangle> out = std::vector<Triangle>();
			int i = 0;
			for (auto face : faces) {
				out.push_back(Triangle(vertices[face[0]], vertices[face[1]], vertices[face[2]], materials[mtl_map[i]].color));
				i++;
			}
			return out;
		}
	};
}

#endif // !MODELLOADER_H

