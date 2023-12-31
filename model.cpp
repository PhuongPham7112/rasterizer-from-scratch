#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

const char vert_prefix[3] = "v ";
const char face_prefix[3] = "f ";
const char vert_texture_prefix[5] = "vt  ";
const char normal_prefix[5] = "vn ";


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
            glm::dvec3 v;
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
            iss >> trash;
            iss >> trash;
            glm::dvec3 vt;
            for (int i = 0; i < 3; i++) iss >> vt[i];
            verts_texture_.push_back(vt);
        }
        else if (!line.compare(0, 3, normal_prefix)) {
            iss >> trash >> trash;
            glm::dvec3 n;
            for (int i = 0; i < 3; i++) iss >> n[i];
            norms_.push_back(n);
        }
    }
    std::cerr << "# v# " << verts_.size() << " #vt " << verts_texture_.size() << " vertex texture idx " << verts_texture_idx_.size() << " f# " << faces_.size() << " vn# " << norms_.size() << std::endl;

    load_texture(filename, "_diffuse.tga", diffusemap);
    diffusemap.flip_vertically();
    load_texture(filename, "_nm.tga", normalmap);
    normalmap.flip_vertically();
    load_texture(filename, "_spec.tga", specularmap);
    specularmap.flip_vertically();
    load_texture(filename, "_glow.tga", glowmap);
    glowmap.flip_vertically();

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

// list of index to vertices making up this face idx
std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::vert_texture_idx(int idx) {
    return verts_texture_idx_[idx];
}

glm::dvec3 Model::normal(int iface) {
    std::vector<int> face = this->face(iface);
    glm::dvec3 world_coords[3];
    for (int j = 0; j < 3; j++) {
        world_coords[j] = this->vert(face[j]);
    }
    return glm::normalize(glm::cross(world_coords[1] - world_coords[0], world_coords[2] - world_coords[0]));
}

glm::dvec3 Model::normal(int iface, int nthvert) {
    int idx = faces_[iface][nthvert];
    return glm::normalize(norms_[idx]);
}

glm::dvec3 Model::vert(int i) {
    return verts_[i];
}

glm::dvec3 Model::vert_texture(int i) {
    return verts_texture_[i];
}

void Model::load_texture(std::string filename, const std::string suffix, TGAImage& img) {
    size_t dot = filename.find_last_of(".");
    if (dot == std::string::npos) return;
    std::string texfile = filename.substr(0, dot) + suffix;
    std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
}

