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
				
				if (token[0] == '#') continue; //Skip comments
				else if (token == "v") parseVertex(stream);
				else if (token == "vt") parseVertexTexture(stream);
				else if (token == "vn") parseVertexNormal(stream);
				else if (token == "f") parseFace(stream);
				else printf("Token not yet supported: %s", token);

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
		//if (!(vertex >> w)) w = 1.0f;
		vertices.push_back(glm::vec3(x, y, z));
	}

	void Model::parseVertexTexture(std::istringstream& texture) {
		float u, v, w;
		texture >> u >> v;
		//if (!(texture >> w)) w = 0.0f;
		vertex_textures.push_back(glm::vec2(u, v));
	}

	void Model::parseVertexNormal(std::istringstream& normal) {
		//May not be normalized
		float x, y, z;
		normal >> x >> y >> z;
		vertex_normals.push_back(glm::vec3(x, y, z));
	}

	void Model::parseFace(std::istringstream& face) {
		// vertex/texture/normal
		std::string v[3];
		face >> v[0] >> v[1] >> v[2];
		//glm::umat3 oface = glm::umat3();
		for (int i = 0; i < 3; i++) {
			int vertex, texture, normal;
			std::istringstream section(v[i]);
			section >> vertex;
			
			if (section.peek() == '/') { 
				section.ignore();
				if (section.peek() != '/') section >> texture;
			}
			else continue;

			if (section.peek() == '/') {
				section.ignore();
				section >> normal;
			}
			else continue;
			//Assume vertex/texture/normal are all available
			vertexIndices.push_back(vertex);
			textureIndices.push_back(texture);
			normalIndices.push_back(normal);
		}
	}


	Model& loadModelFromFile(std::string file_name) {

	}
}