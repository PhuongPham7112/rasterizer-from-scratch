#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include <glm/glm.hpp>

class Model {
private:
	std::vector<glm::dvec3> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int> > verts_texture_idx_;
	std::vector<glm::dvec3> verts_texture_;

public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int nvertTex();
	glm::dvec3 vert(int i);
	glm::dvec3 vert_texture(int i);
	std::vector<int> face(int idx);
	std::vector<int> vert_texture_idx(int idx);
};

#endif //__MODEL_H__