#pragma once

#include "util.hpp"

namespace demo {

struct GObjVertex : public IVertex {
	vec4 pos;
	vec2 uv;
    vec3 normal;
    vec3 color;

	GObjVertex() { }
	GObjVertex(vec4 p, vec2 u, vec3 n, vec3 c) : pos(p), uv(u), normal(n), color(c) { }

	void berp(vec3 bary, GObjVertex v0, GObjVertex v1, GObjVertex v2, float d) {
        pos.x = b_interpolate(bary, vec3(v0.pos.x, v1.pos.x, v2.pos.x)) * d;
        pos.y = b_interpolate(bary, vec3(v0.pos.y, v1.pos.y, v2.pos.y)) * d;
        pos.z = b_interpolate(bary, vec3(v0.pos.z, v1.pos.z, v2.pos.z)) * d;

		uv.x = b_interpolate(bary, vec3(v0.uv.x, v1.uv.x, v2.uv.x)) * d;
		uv.y = b_interpolate(bary, vec3(v0.uv.y, v1.uv.y, v2.uv.y)) * d;

		normal.x = b_interpolate(bary, vec3(v0.normal.x, v1.normal.x, v2.normal.x)) * d;
		normal.y = b_interpolate(bary, vec3(v0.normal.y, v1.normal.y, v2.normal.y)) * d;
		normal.z = b_interpolate(bary, vec3(v0.normal.z, v1.normal.z, v2.normal.z)) * d;

        color.x = b_interpolate(bary, vec3(v0.color.x, v1.color.x, v2.color.x)) * d;
		color.y = b_interpolate(bary, vec3(v0.color.y, v1.color.y, v2.color.y)) * d;
		color.z = b_interpolate(bary, vec3(v0.color.z, v1.color.z, v2.color.z)) * d;
	}

	void lerp(GObjVertex a, GObjVertex b, float alpha) {
		pos = l_interpolate(a.pos, b.pos, alpha);
		uv = l_interpolate(a.uv, b.uv, alpha); 
        normal = l_interpolate(a.normal, b.normal, alpha);
        color = l_interpolate(a.color, b.color, alpha);
	}

	std::string to_string() {
		std::stringstream ss;
		ss << glm::to_string(pos) << " " << glm::to_string(uv);
		return ss.str();
	}

	GObjVertex operator*(float z) {
		pos *= z;
		uv *= z;
        normal *= z;
        color *= z;
		return *this;
	}

	GObjVertex operator/(float z) {
		pos /= z;
		uv /= z;
        normal /= z;
        color /= z;
		return *this;
	}
};

class GObj {
public:
    GObj(std::string filename) {
        std::ifstream s(filename);
        std::stringstream buffer;
        buffer << s.rdbuf();

        std::string line;

        while(std::getline(buffer, line)) {
            std::vector<std::string> vec = split_string(line, ' ');
            if(line.substr(0, 2) == "v ") {
                // vertex
                vec.erase(vec.cbegin());

                if(vec.size() < 3) continue;

                vec4 position;
                position.x = std::atof(vec.at(0).c_str());
                position.y = std::atof(vec.at(1).c_str());
                position.z = std::atof(vec.at(2).c_str());
                position.w = (vec.size() < 4) ? 1 : std::atof(vec.at(3).c_str());

                positions.push_back(position);
            }
            else if(line.substr(0, 3) == "vt ") {
                // uv coord
                vec.erase(vec.cbegin());
                
                if(vec.size() != 2) continue;

                vec2 uv;

                uv.x = std::atof(vec.at(0).c_str());
                uv.y = std::atof(vec.at(1).c_str());

                uvs.push_back(uv);
            }
            else if(line.substr(0, 3) == "vn ") {
                // normal
                vec.erase(vec.cbegin());
                
                if(vec.size() != 3) continue;

                vec3 normal;

                normal.x = std::atof(vec.at(0).c_str());
                normal.y = std::atof(vec.at(1).c_str());
                normal.z = std::atof(vec.at(2).c_str());

                normals.push_back(normal);
            }
            else if(line.substr(0, 2) == "f ") {
                // face
                vec.erase(vec.cbegin());

                if(vec.size() < 3) continue;

                for(auto v : vec) {
                    std::vector<std::string> data = split_string(v, '/');

                    GObjVertex vertex(vec4(0), vec2(0), vec3(0), vec3(0));

                    switch(data.size()) {
                    case 0:
                        continue;

                    case 1: { // only vertex
                        int idx = std::atoi(data.at(0).c_str());
                        if(idx > positions.size()) break;

                        if(idx-- == -1) idx = positions.size() - 1;
                        
                        vertex.pos = positions.at(idx);
                        break;
                    }

                    case 2: { // vertex/uv
                        int p_idx = std::atoi(data.at(0).c_str()),
                            t_idx = std::atoi(data.at(1).c_str());
                        if(p_idx > positions.size() ||
                            t_idx > uvs.size()) break;

                        if(p_idx-- == -1) p_idx = positions.size() - 1;
                        if(t_idx-- == -1) t_idx = uvs.size() - 1;
                        
                        vertex.pos = positions.at(p_idx);
                        vertex.uv = uvs.at(t_idx);

                        break;
                    }

                    case 3: { // vertex/uv/normal OR vertex//normal
                        int p_idx = std::atoi(data.at(0).c_str());
                    
                        if(p_idx-- == -1) p_idx = positions.size() - 1;
                        if(p_idx > positions.size()) {
                            std::cout << "skip p_idx";
                            break;
                        }

                        if(data.at(1).empty()) { // no uv
                            int n_idx = std::atoi(data.at(2).c_str());

                            if(n_idx-- == -1) n_idx = normals.size() - 1;
                            if(n_idx > normals.size()) {
                                std::cout << "skip n_idx";
                                break;
                            }
                            
                            vertex.pos = positions.at(p_idx);
                            vertex.normal = normals.at(n_idx);
                        } else { // vertex/uv/normal
                            int t_idx = std::atoi(data.at(1).c_str()),
                                n_idx = std::atoi(data.at(2).c_str());

                            if(t_idx-- == -1) t_idx = uvs.size() - 1;
                            if(n_idx-- == -1) n_idx = normals.size() - 1;

                            if(t_idx > uvs.size() || n_idx > normals.size()) {
                                std::cout << "skip t_idx n_idx";
                                break;
                            }
                            
                            vertex.pos = positions.at(p_idx);
                            vertex.uv = uvs.at(t_idx);
                            vertex.normal = normals.at(n_idx);
                        }

                        break;
                    }
                    }
                    
                    vertices.push_back(vertex);
                    indices.push_back(vertices.size()-1);
                }
            }
        }

        std::cout << "loaded object. " << vertices.size() << " vertices.\n";
    }

    GTriangleList<GObjVertex> get_triangle_list() {
        return GTriangleList<GObjVertex>(vertices, indices);
    }

    std::vector<GObjVertex> vertices;
    std::vector<std::size_t> indices;

private:
    std::vector<vec4> positions;
    std::vector<vec2> uvs;
    std::vector<vec3> normals;
};

}