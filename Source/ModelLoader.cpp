#include "../Include/ModelLoader.h"
#define EQUAL(s1, s2) strcmp(s1, s2) == 0

std::string trim(const std::string& str, const std::string& whitespace = " \t\n\r") {
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

namespace model {
	Model::Model(std::string file_name) {
		std::string token;
		
		std::ifstream file;
		file.open(file_name);

		if (file.is_open()) {
			std::string line;
			
			while (std::getline(file, line)) {
				std::istringstream stream(line);
				stream >> token;
				
				//Skip comments
				if (token[0] == '#') continue;
				if (token == "v") parseVertex(stream);
			}
			file.close();
		}
		else {
			std::cerr << "Unable to open file: " << file_name << std::endl;
		}
	}

	//Tested. Functioning correctly.
	void Model::parseVertex(std::istringstream& vertex) {
		float x, y, z, w;
		vertex >> x >> y >> z;
		//if (!(vertex >> w)) w = 1.0;
		vertices.push_back(glm::vec3(x, y, z));
	}

	void Model::parseUV(std::istringstream& UV) {
		float u, v, w;
		UV >> u >> v;
		//if (!(vertex >> w)) w = 1.0;
		uvs.push_back(glm::vec2(u, v));
	}

	Model& loadModelFromFile(std::string file_name) {

	}
}