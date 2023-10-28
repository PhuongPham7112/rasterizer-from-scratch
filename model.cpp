#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

const char vert_prefix[3] = "v ";
const char face_prefix[3] = "f ";
const char vert_texture_prefix[5] = "vt  ";

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, vert_prefix)) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, face_prefix)) {
            std::vector<int> f;
            std::vector<int> vt;
            int itrash, idx, tex_idx;
            iss >> trash;
            while (iss >> idx >> trash >> tex_idx >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                tex_idx--;
                f.push_back(idx);
                vt.push_back(tex_idx);
            }
            faces_.push_back(f);
            verts_texture_idx_.push_back(vt);
        } else if (!line.compare(0, 4, vert_texture_prefix)) {
            // vt = a list of vertex coordinates
            iss >> trash;
            iss >> trash;
            Vec3f vt;
            for (int i = 0; i < 3; i++) iss >> vt[i];
            verts_texture_.push_back(vt);
        }
    }
    std::cerr << "# v# " << verts_.size() << " #vt " << verts_texture_.size() << " vertex texture idx " << verts_texture_idx_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

int Model::nvertTex() {
    return (int)verts_texture_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::vert_texture_idx(int idx) {
    return verts_texture_idx_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::vert_texture(int i) {
    return verts_texture_[i];
}