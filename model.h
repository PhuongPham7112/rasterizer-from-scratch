#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include <glm/glm.hpp>
#include "tgaimage.h"

class Model {
private:
	std::vector<glm::dvec3> verts_;
	std::vector<glm::dvec3> norms_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int> > verts_texture_idx_;
	std::vector<glm::dvec3> verts_texture_;

public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int nvertTex();
	TGAImage diffusemap{};         // diffuse color texture
	TGAImage normalmap{};          // normal map texture
	TGAImage specularmap{};        // specular map texture
	TGAImage glowmap{};        // specular map texture
	glm::dvec3 vert(int i);
	glm::dvec3 vert_texture(int i);
	glm::dvec3 normal(int iface);
	glm::dvec3 normal(int iface, int nthvert);
	std::vector<int> face(int idx);
	std::vector<int> vert_texture_idx(int idx);
	void load_texture(std::string filename, const std::string suffix, TGAImage& img);
};

#endif //__MODEL_H__