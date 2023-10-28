#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int> > verts_texture_idx_;
	std::vector<Vec3f> verts_texture_;

public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int nvertTex();
	Vec3f vert(int i);
	Vec3f vert_texture(int i);
	std::vector<int> face(int idx);
	std::vector<int> vert_texture_idx(int idx);
};

#endif //__MODEL_H__