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
		std::string token = nullptr;
		
		std::ifstream file;
		file.open(file_name);

		if (file.is_open()) {
			std::string line;
			
			while (std::getline(file, line)) {
				std::istringstream stream(line);
				stream >> token;
				
				//Skip comments
				if (token[0] == '#') continue;
				if (token == "v") parseVertex(line);
			}
			file.close();
		}
		else {
			std::cerr << "Unable to open file: " << file_name << std::endl;
		}
	}

	void Model::parseVertex(std::string vertex_line) {
		float x, y, z, w;
		std::istringstream vertex_stream(vertex_line);
		vertex_stream >> x >> y >> z >> w;
		std::cout << x << std::endl;
		std::cout << y << std::endl;
		std::cout << z << std::endl;
		std::cout << w << std::endl;
	}

	Model& loadModelFromFile(std::string file_name) {

	}
}