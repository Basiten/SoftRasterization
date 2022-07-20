#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_() {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        // 读取 vertices
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            glm::vec3 v;
             iss >> v.x;
             iss >> v.y;
             iss >> v.z;
            verts_.push_back(v);
        }
        // 读取 texCoords
        if (!line.compare(0, 2, "vt")) {
            iss >> trash;
            iss >> trash;
            glm::vec2 vt;
            iss >> vt.x;
            iss >> vt.y;
            texcoords_.push_back(vt);
        }
        // 读取facet和vertices, texCoords的对应关系
        else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<int> fTexCoords;
            int iTexCoords, idx, itrash;
            iss >> trash;
            while (iss >> idx >> trash >> iTexCoords >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                iTexCoords--;
                f.push_back(idx);
                fTexCoords.push_back(iTexCoords);
            }
            faces_.push_back(f);
            facesToTexCoords_.push_back(fTexCoords);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() const{
    return (int)verts_.size();
}

int Model::nfaces() const{
    return (int)faces_.size();
}

int Model::nTexCoords() const {
    return (int)texcoords_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::faceTexCoords(int idx) {
    return facesToTexCoords_[idx];
}

glm::vec3 Model::vert(int i) {
    return verts_[i];
}

glm::vec2 Model::texCoords(int i){
    return texcoords_[i];
}
