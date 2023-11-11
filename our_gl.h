#include "tgaimage.h"
#include <glm/glm.hpp>

extern double ks = 0.6; // specular const
extern double kd = 1.0; // diffuse const
extern double ka = 3.0; // ambient const
extern glm::dmat4 ModelView_mat;
extern glm::dmat4 Projection_mat;
extern glm::dmat4 Viewport_mat;

struct IShader {
    virtual ~IShader() = default;
    virtual glm::dvec3 vertex(int iface, int nthvert) = 0; // function to transform the coordinates of the vertices and prepare data for the fragment shader.
    virtual bool fragment(glm::dvec3 baryCoord, TGAImage& tex_image, TGAImage& nm_image, TGAColor& color) = 0; // function to determine the color of the current pixel and discard vertices
};

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);

void viewport(double x, double y, double w, double h, double d);

void lookAt(glm::dvec3 eye, glm::dvec3 center, glm::dvec3 up);

void projection(double cameraZ);

void triangle(glm::dvec3* pts, IShader& shader, TGAImage& image, TGAImage& tex_image, TGAImage& nm_image, double* zbuffer);

glm::dvec3 barycentric(glm::dvec3 A, glm::dvec3 B, glm::dvec3 C, glm::dvec3 P);
