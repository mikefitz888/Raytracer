//#define DEBUG
#include "../Include/ModelLoader.h"
#include "../Include/Raytracer.h"
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
	void Scene::getTriangles(std::vector<Triangle>& triangles) {
		for (auto model : models) {
			for (auto triangle : *(model->getFaces())) {
				triangles.push_back(triangle);
			}
		}
	}
    Model::Model(std::vector<Triangle> triangles) {
        this->triangles = triangles;
        modified = false;
        this->CalculateBoundingVolume();
        //this->generateOctree();
    }

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
				//else if (token == "mtllib") parseMaterialLib(stream);
				//else if (token == "usemtl") parseUseMaterial(stream);
				else std::cout << "Token not supported: '" << token << "'." << std::endl;

			}
			file.close();
		}
		else {
			std::cerr << "Unable to open file: " << file_name << std::endl;
		}
        this->getFaces();
        this->CalculateBoundingVolume();
        //this->generateOctree();
	}

	//Tested. Functioning correctly.
	void Model::parseVertex(std::istringstream& vertex) {
		float x, y, z, w;
		vertex >> x >> y >> z;
		//if (!(vertex >> w)) w = 1.0f;
		vertices.emplace_back(x, y, z);
#ifdef DEBUG
		printf("Added vertex: (%f, %f, %f) \n", x, y, z);
#endif
	}

	void Model::parseVertexTexture(std::istringstream& texture) {
		float u, v, w;
		texture >> u >> v;
		//if (!(texture >> w)) w = 0.0f;
		vertex_textures.emplace_back(u, v);
#ifdef DEBUG
		printf("Added vertex_texture: (%f, %f) \n", u, v);
#endif
	}

	void Model::parseVertexNormal(std::istringstream& normal) {
		//May not be normalized
		float x, y, z;
		normal >> x >> y >> z;
		vertex_normals.emplace_back(x, y, z);
#ifdef DEBUG
		printf("Added vertex_normal: (%f, %f, %f) \n", x, y, z);
#endif
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
			vertexIndices.push_back(vertex-1);
			textureIndices.push_back(texture-1);
			normalIndices.push_back(normal-1);
#ifdef DEBUG
			printf("Added face: (%d, %d, %d) \n", vertex, texture, normal);
#endif
		}
	}

    void Model::CalculateBoundingVolume() {
        for (auto &t : this->triangles) {
            this->bounding_box.min = glm::min(glm::min(this->bounding_box.min, t.v0), glm::min(t.v1, t.v2));
            this->bounding_box.max = glm::max(glm::max(this->bounding_box.max, t.v0), glm::max(t.v1, t.v2));
        }
    }
    void Model::generateOctree() {
        this->octree = new Octree(this->bounding_box, nullptr);
        this->octree->fill(*this->getFaces());
        this->octree->split();
        this->use_optimising_structure = true;
    }

    bool Model::hasOctree() {
        return (this->octree != nullptr);
    }

    Octree* Model::getOctree() {
        return this->octree;
    }


	/*void Model::parseMaterialLib(std::istringstream& lib){
		std::string file_name;
		lib >> file_name;
		std::ifstream file;
		file.open(file_name);

		if(file.is_open()){
			std::string line, token;
			while (std::getline(file, line)) {
				std::istringstream stream(line);
				stream >> token;

				Material m;
				if (materials.size()) { m = getActiveMaterial(); }
				if(token[0] == '#') continue; //skip comments
				else if (token == "newmtl") {
					std::string name;
					stream >> name;
					materials.push_back(Material());
					material_map.insert(std::pair<std::string, unsigned int>(name, materials.size()-1));
				}
				else if (token == "Ka") stream >> m.ambient_color.x >> m.ambient_color.y >> m.ambient_color.z; //ambient color
				else if (token == "Kd") stream >> m.diffuse_color.x >> m.diffuse_color.y >> m.diffuse_color.z; //diffuse color
				else if (token == "Ks") stream >> m.specular_color.x >> m.specular_color.y >> m.specular_color.z; //specular color
				else if (token == "Ns") stream >> m.specular_exponent;
				else if (token == "Tr") stream >> m.transparency;
				//else if (token == "map_Ka") parseAmbientMap(stream);
				//else if (token == "map_Kd") parseDiffuseMap(stream);
				//else if (token == "map_Ks") parseSpecularMap(stream);
				//else if (token == "map_bump") parseBumpMap(stream);
			}
		}else{
			std::cerr << "Failed to open material: " << file_name << std::endl;
		}
	}

	void Model::parseUseMaterial(std::istringstream& material){
		std::string material_name;
		material >> material_name;
		active_material = material_map[material_name];
	}*/


    // ******************************************************************** //
    // OCTREE

    // Constructor
    Octree::Octree(AABB bounding_box, Octree* parent) {
        this->bounding_box = bounding_box;
        this->parent = parent;
        if (this->parent == nullptr) {
            depth = 0;
        } else {
            depth = this->parent->getDepth() + 1;
        }
        init();
    }

    // Initialise
    void Octree::init() {
        for (int i = 0; i < 8; i++) {
            children[i] = nullptr;
        }
    }
    
    /*
        Will add any triangles that fit within the Octrees bounding box
    */
    void Octree::fill(const std::vector<Triangle>& triangles) {
        for (auto t : triangles) {
            if (Octree::triangle_aabb_intersection(t, this->bounding_box)) {
                this->addTriangle(t);
            }
        }
        if (this->triangles.size() > 0) {
            /*std::cout << "Octree segment filled. " << std::endl
                << "\tDEPTH: " << depth << std::endl
                << "\tTRIANGLES: " << this->triangles.size() << std::endl;*/
        }
    }

    void Octree::split() {
        // Only split if we are not too deep into the tree
        if (depth >= Octree::MAX_OCTREE_DEPTH) { return; }

        // Create 8 child sections
        glm::vec3 mid_point = (this->bounding_box.min+ this->bounding_box.max)*0.5f;

        int i = 0; 
        for (float x = 0; x < 1; x += 0.5) {
            for (float y = 0; y < 1; y += 0.5) {
                for (float z = 0; z < 1; z += 0.5) {
                    // Create section bounding box
                    AABB aabb;
                    aabb.min.x = (x == 0) ? this->bounding_box.min.x : mid_point.x;
                    aabb.min.y = (y == 0) ? this->bounding_box.min.y : mid_point.y;
                    aabb.min.z = (z == 0) ? this->bounding_box.min.z : mid_point.z;

                    aabb.max.x = (x == 0) ? mid_point.x : this->bounding_box.max.x;
                    aabb.max.y = (y == 0) ? mid_point.y : this->bounding_box.max.y;
                    aabb.max.z = (z == 0) ? mid_point.z : this->bounding_box.max.z;

                    // Create new octree and fill with triangles (Then split new octree)
                    Octree* octree = new Octree(aabb, this);
                    octree->fill(this->triangles);

                    if (octree->getTriangleCount() > 0) {
                        // If the segment has triangles, keep reference
                        this->children[i] = octree;
                        octree->split();
                    } else {

                        // segment is empty, delete it
                        this->children[i] = nullptr;
                        delete octree;
                    }

                    // Increment i
                    i++;
                }
            }
        }
    }


    /*
        PROJECT AND TRIANGLE_AABB functions taken from:
        http://stackoverflow.com/questions/17458562/efficient-aabb-triangle-intersection-in-c-sharp
       
        (ported to C++)
    */
    void Octree::project(std::vector<glm::vec3> points, glm::vec3 axis, double &min, double &max) {

        min = 1000000000000000000.0f;
        max = -100000000000000000.0f;
        for(auto p : points) {
            double val = glm::dot(axis, p);
            if (val < min) min = val;
            if (val > max) max = val;
        }
    }

    // returns whether a triangle interects with an AABB
    /*
        This is done by attempting to find a separation plane between the two sets of
        vertices
    */
    bool Octree::triangle_aabb_intersection(Triangle& t, AABB& box) {

        // Calculate AABB for triangle
        AABB aabb;
        aabb.min = glm::min(t.v0, glm::min(t.v1, t.v2));
        aabb.max = glm::max(t.v0, glm::max(t.v1, t.v2));
        return aabb_aabb_intersection(aabb, box);
    }

    bool Octree::aabb_aabb_intersection(AABB& a, AABB& b) {
        return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
            (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
            (a.min.z <= b.max.z && a.max.z >= b.min.z);
    }
    /*bool Octree::triangle_aabb_intersection(Triangle& t, AABB& box) {
        double triangleMin, triangleMax;
        double boxMin, boxMax;
        auto box_vertices = {
            glm::vec3(box.min.x, box.min.y, box.min.x),
            glm::vec3(box.min.x, box.min.y, box.max.x),
            glm::vec3(box.min.x, box.max.y, box.min.x),
            glm::vec3(box.min.x, box.max.y, box.max.x),
            glm::vec3(box.max.x, box.min.y, box.min.x),
            glm::vec3(box.max.x, box.min.y, box.max.x),
            glm::vec3(box.max.x, box.max.y, box.min.x),
            glm::vec3(box.max.x, box.max.y, box.max.x)
        };

        // Test the box normals (x-, y- and z-axes)
        glm::vec3 boxNormals[3];
        boxNormals[0] = glm::vec3(1, 0, 0);
        boxNormals[1] = glm::vec3(0, 1, 0);
        boxNormals[2] = glm::vec3(0, 0, 1);

        for (int i = 0; i < 3; i++) {
            glm::vec3 n = boxNormals[i];
            project({t.v0, t.v1, t.v2}, boxNormals[i], triangleMin, triangleMax);
            if (triangleMax < box.min[i] || triangleMin > box.max[i])
                return false; // No intersection possible.
        }

        // Test the triangle normal
        double triangleOffset =   glm::dot(t.normal, t.v0);
        project(box_vertices, t.normal, boxMin, boxMax);
        if (boxMax < triangleOffset || boxMin > triangleOffset)
            return false; // No intersection possible.

                          // Test the nine edge cross-products
        glm::vec3 triangleEdges[3];
        triangleEdges[0] = t.v0 - t.v1;
        triangleEdges[1] = t.v1 - t.v2;
        triangleEdges[2] = t.v2 - t.v0;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++) {
                // The box normals are the same as it's edge tangents
                //IVector axis = triangleEdges[i].Cross(boxNormals[j]);
                glm::vec3 axis = glm::cross(triangleEdges[i], boxNormals[j]);


                project(box_vertices, axis, boxMin, boxMax);
                project({t.v0, t.v1, t.v2}, axis, triangleMin, triangleMax);
                if (boxMax < triangleMin || boxMin > triangleMax)
                    return false; // No intersection possible
            }

        // No separating axis found.
        return true;

    }*/

    void Octree::getIntersectingSections(glm::vec3 ray_orig, glm::vec3 ray_inv_dir, std::vector<std::vector<Triangle>*> &storage) {

        // Exit if there are no triangles
        if (this->getTriangleCount() == 0) {
            return;
        }

        // Check if ray intersects with us
        Ray ray(ray_orig, ray_inv_dir);
        if (!ray.ray_aabb_intersect(ray_orig, ray_inv_dir, this->bounding_box)) {
            return;
        }

        // Check if we have any children
        int child_count = 0;
        for (int i = 0; i < 8; i++) {
            if (children[i] != nullptr) {
                child_count++;
            }
        }

        // If child count is 0, we are at the lowest level:
        if (child_count == 0) {
            // Add my vertices to the list
            storage.push_back(this->getTriangles());
        } else {
            // Pass onto children
            for (int i = 0; i < 8; i++) {
                if (children[i] != nullptr) {
                    children[i]->getIntersectingSections(ray_orig, ray_inv_dir, storage);
                }
            }
        }

    }
}