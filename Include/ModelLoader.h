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
		std::vector<glm::vec3> vertices, normals;
		std::vector<glm::vec2> uvs;

	private:
		void parseVertex(std::istringstream& vertex);
		void parseUV(std::istringstream& UV);
	public:
		Model(std::string file_name);
	};

	Model& loadModelFromFile(std::string file_name);
}

#endif // !MODELLOADER_H

