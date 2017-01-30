#ifndef MODELLOADER_H
#define MODELLOADER_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_integer.hpp>
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
		std::vector<unsigned int> vertexIndices, textureIndices, normalIndices;
		std::vector<glm::vec3> vertices, vertex_normals;
		std::vector<glm::vec2> vertex_textures; //UVs

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
			//faces.push_back(glm::uvec3(a, b, c));
			//mtl_map.push_back(color);
		}

		inline void addMaterial(glm::vec3 color) {
			materials.push_back(Material(color));
		}

		//Convert to Triangles for raytracer
		inline std::vector<Triangle>& getFaces() {
			std::vector<Triangle> out = std::vector<Triangle>();
			for(int i = 0; i < vertexIndices.size(); i+=3){
				out.push_back(Triangle(
					vertices[vertexIndices[i]],
					vertices[vertexIndices[i+1]],
					vertices[vertexIndices[i+2]],
					vertex_textures[textureIndices[i]],
					vertex_textures[textureIndices[i+1]],
					vertex_textures[textureIndices[i+2]],
					vertex_normals[normalIndices[i]],
					vertex_normals[normalIndices[i+1]],
					vertex_normals[normalIndices[i+2]]
				));
			}
			return out;
		}
	};
}

#endif // !MODELLOADER_H

