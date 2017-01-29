#ifndef MODELLOADER_H
#define MODELLOADER_H
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

namespace model {
	class Model {
		std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
		std::vector<glm::vec3> temp_vertices, temp_normals;
		std::vector<glm::vec2> temp_uvs;

	private:
		void parseVertex(std::string vertex_stream);
	public:
		Model(std::string file_name);
	};

	Model& loadModelFromFile(std::string file_name);
}

#endif // !MODELLOADER_H

