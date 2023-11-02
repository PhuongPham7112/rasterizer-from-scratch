#include "tgaimage.h"
#include <glm/glm.hpp>

extern glm::dmat4 ModelView_mat;
extern glm::dmat4 Projection_mat;
extern glm::dmat4 Viewport_mat;

struct IShader {
    virtual ~IShader();
    virtual glm::ivec3 vertex(int iface, int nthvert) = 0;
    virtual bool fragment(glm::vec3 bar, TGAColor& color) = 0;
};

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);

void viewport(double x, double y, double w, double h, double d);

void lookAt(glm::dvec3 eye, glm::dvec3 center, glm::dvec3 up);

void projection(double cameraZ);

void triangle(glm::dvec3* pts, glm::dvec3* texture_coords, double* zbuffer, TGAImage& image, TGAImage& tex_image, double intensity);

glm::dvec3 barycentric(glm::dvec3 A, glm::dvec3 B, glm::dvec3 C, glm::dvec3 P);
