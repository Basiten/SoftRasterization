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
        // ��ȡ vertices
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++) iss >> v.raw[i];
            verts_.push_back(v);
        }
        // ��ȡ texCoords
        if (!line.compare(0, 2, "vt")) {
            iss >> trash;
            iss >> trash;
            Vec2f vt;
            for (int i = 0; i < 2; i++) iss >> vt.raw[i];
            texcoords_.push_back(vt);
        }
        // ��ȡfacet��vertices�Ķ�Ӧ��ϵ
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

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec2f Model::texCoords(int i){
    return texcoords_[i];
}