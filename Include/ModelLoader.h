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

	class Material {
	public:
		glm::vec3 ambient_color, diffuse_color, specular_color;
		float specular_exponent, transparency;
		bitmap_image* ambient_map, diffuse_map, specular_map, bump_map; 
		inline Material() {};
	};

	class Model {
		std::vector<Material> materials;
		std::map<std::string, unsigned int> material_map;
		unsigned int active_material;

		std::vector<unsigned int> vertexIndices, textureIndices, normalIndices;
		std::vector<glm::vec3> vertices, vertex_normals;
		std::vector<glm::vec2> vertex_textures; //UVs
		std::vector<Triangle> triangles;
		std::vector<int> mtl_map;

	private:
		void parseVertex(std::istringstream& vertex);
		void parseVertexNormal(std::istringstream& vertex_normal);
		void parseVertexTexture(std::istringstream& vertex_texture);
		void parseFace(std::istringstream& face);
		void parseUseMaterial(std::istringstream& material);
		void parseMaterialLib(std::istringstream& lib);
	public:
		Model(std::string file_name);
		inline Model() {};

		inline Material& getActiveMaterial(){
			return materials[active_material];
		}

		//Convert to Triangles for raytracer
		inline std::vector<Triangle>* getFaces() {
			triangles.clear();
			for(int i = 0; i < vertexIndices.size(); i+=3){
				triangles.emplace_back(
					vertices[vertexIndices[i]],
					vertices[vertexIndices[i+1]],
					vertices[vertexIndices[i+2]],
					vertex_textures[textureIndices[i]],
					vertex_textures[textureIndices[i+1]],
					vertex_textures[textureIndices[i+2]],
					vertex_normals[normalIndices[i]],
					vertex_normals[normalIndices[i+1]],
					vertex_normals[normalIndices[i+2]]
				);
			}
			return &triangles;
		}
	};
}

#endif // !MODELLOADER_H

