#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "our_gl.h"
#include "tgaimage.h"
#include "model.h"
#include <glm/gtc/matrix_access.hpp>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;

glm::dvec3 camera(-2, 0, 5);
glm::dvec3 light_dir(-1, 1, 1);
glm::dvec3 cameraTarget(0, 0, 0);

// printing
void printDVec3(const glm::dvec3& vec) {
    std::cout << "glm::dvec3(" << vec.x << ", " << vec.y << ", " << vec.z  << ")" << std::endl;
}

void printDVec4(const glm::dvec4& vec) {
    std::cout << "glm::dvec4(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")" << std::endl;
}

void printDMat4(const glm::dmat4& mat) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << mat[i][j];
            if (j < 3) {
                std::cout << ", ";
            }
            else {
                std::cout << std::endl;
            }
        }
    }
}

struct GouraudShader : public IShader {
    int varying_iface;
    glm::dvec3 varying_view;
    glm::dvec3 varying_uvCoords[3];
    glm::dvec3 varying_fragPos[3];
    glm::dmat3 varying_normal;
    glm::dmat4 uniform_M;
    glm::dmat4 uniform_invM;

    double ks = 0.4;
    double ka = 0.1;
    double kd = 0.6;

    virtual glm::dvec3 vertex(int iface, int nthvert) override {
        varying_iface = iface;
        varying_normal[nthvert] = glm::normalize(glm::dvec3(uniform_invM * glm::dvec4(model->normal(iface, nthvert), 0.0)));
        varying_uvCoords[nthvert] = model->vert_texture(model->vert_texture_idx(iface)[nthvert]);
        
        // rasterize
        glm::dvec3 v = model->vert(model->face(iface)[nthvert]);
        glm::dvec4 aug_coords(v.x, v.y, v.z, 1.0);
        glm::dvec4 aug_mat = Viewport_mat * Projection_mat * ModelView_mat * aug_coords;

        // make smoother result
        glm::dvec3 result;
        result.x = int(aug_mat[0] / aug_mat[3]);
        result.y = int(aug_mat[1] / aug_mat[3]);
        result.z = aug_mat[2] / aug_mat[3];

        // variables
        varying_fragPos[nthvert] = result;
        varying_view = glm::normalize(camera - varying_fragPos[nthvert]);
        return result;
    }

    virtual bool fragment(glm::dvec3 baryCoord, TGAImage& tex_image, TGAImage& nm_image, TGAImage& spec_image, TGAColor& color) override {
        glm::dvec3 uv = varying_uvCoords[0] * baryCoord[0] + varying_uvCoords[1] * baryCoord[1] + varying_uvCoords[2] * baryCoord[2];
        
        // normal from map
        TGAColor nm_color = nm_image.get((int)(uv[0] * nm_image.get_width()), (int)(uv[1] * nm_image.get_height()));
        glm::dvec3 norm;
        for (int i = 0; i < 3; i++) {
            norm[2 - i] = (double)nm_color[i] / 255.0 * 2.0 - 1.0;
        }
        glm::dvec3 n_map = glm::normalize(glm::dvec3(uniform_invM * glm::dvec4(norm, 0.0)));

        // normal and light vector
        glm::dvec3 l = glm::normalize(glm::dvec3(uniform_M * glm::dvec4(light_dir, 0.0)));
        glm::dvec3 bn = glm::normalize(varying_normal * baryCoord);

        // darboux basis
        glm::dmat3 mat_A;
        mat_A[0] = varying_fragPos[1] - varying_fragPos[0];
        mat_A[1] = varying_fragPos[2] - varying_fragPos[0];
        mat_A[2] = bn;
        mat_A = glm::transpose(mat_A);

        glm::dmat3 mat_AInv = glm::inverse(mat_A);
        glm::dvec3 i = -mat_AInv * glm::dvec3(varying_uvCoords[1][0] - varying_uvCoords[0][0], varying_uvCoords[2][0] - varying_uvCoords[0][0], 0.0);
        glm::dvec3 j = -mat_AInv * glm::dvec3(varying_uvCoords[1][1] - varying_uvCoords[0][1], varying_uvCoords[2][1] - varying_uvCoords[0][1], 0.0);

        glm::dmat3 mat_B;
        mat_B[0] = glm::normalize(i);
        mat_B[1] = glm::normalize(j);
        mat_B[2] = bn;
        glm::dvec3 n = n_map;
        
        // diffuse
        TGAColor tex_color = tex_image.get((int)(uv[0] * tex_image.get_width()), (int)(uv[1] * tex_image.get_height()));
        double diffuse_intensity = std::max(0.0, glm::dot(n, l));

        // specular
        TGAColor spec_color = spec_image.get((int)(uv[0] * spec_image.get_width()), (int)(uv[1] * spec_image.get_height()));
        glm::dvec3 reflection = glm::normalize(glm::reflect(l, n));
        double cos_angle = glm::max(glm::dot(reflection, varying_view), 0.0);
        double spec_intensity = glm::pow(cos_angle, 5.0 + spec_color[0]/1.0);

        // final color
        double ambient_intensity = 1.0;
        color = tex_color;

        // clamp each color
        for (int i = 0; i < 3; i++) {
            color[i] = std::min<double>(tex_color[i] * (ka * ambient_intensity + ks * spec_intensity + kd * diffuse_intensity), 255.0);
        }
        return false;
    }
};

int main(int argc, char** argv) {
    if (2 == argc) {
        model = new Model(argv[1]);
    }
    else {
        model = new Model("obj/african_head.obj");
    }

    double* zbuffer = new double[(width * height)];
    for (int i = (width * height); i--; zbuffer[i] = std::numeric_limits<double>::min());

    TGAImage outImage(width, height, TGAImage::RGB);
    outImage.flip_vertically();

    TGAImage textureImage; 
    textureImage.read_tga_file("obj/african_head_diffuse.tga");
    textureImage.flip_vertically();

    TGAImage normalImage;
    normalImage.read_tga_file("obj/african_head_nm.tga");
    normalImage.flip_vertically();

    TGAImage specImage;
    specImage.read_tga_file("obj/african_head_spec.tga");
    specImage.flip_vertically();

    // all transformation matrices
    glm::dvec3 cameraEye = glm::normalize(cameraTarget - camera);
    projection(camera.z); // projection matrix
    viewport(static_cast<double>(width) / 8.0, static_cast<double>(height) / 8.0, static_cast<double>(width) * 0.75, static_cast<double>(height) * 0.75, depth); // viewport matrix
    lookAt(cameraEye, camera, glm::dvec3(0.0, 1.0, 0.0)); // modelview matrix

    // populate face
    GouraudShader shader;
    shader.uniform_M = Projection_mat * ModelView_mat;
    shader.uniform_invM = glm::inverse(shader.uniform_M);

    for (int i = 0; i < model->nfaces(); i++) {
        glm::dvec3 pts[3];
        for (int j = 0; j < 3; j++) {
            pts[j] = shader.vertex(i, j);
        }
        triangle(pts, shader, outImage, textureImage, normalImage, specImage, zbuffer);
    }

    outImage.write_tga_file("output.tga");

    delete model;
    delete[] zbuffer;
    return 0;
}
