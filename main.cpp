#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "our_gl.h"
#include "tgaimage.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;

glm::dvec3 camera(1, 0, 4);
glm::dvec3 light_dir(0, 0, 1);
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
    glm::dvec3 varying_intensity;
    glm::dvec3 varying_uvCoords[3];
    glm::dmat4 uniform_M;
    glm::dmat4 uniform_invM;
    virtual glm::dvec3 vertex(int iface, int nthvert) override {
        glm::dvec3 n = glm::normalize(glm::dvec3(uniform_invM * glm::dvec4(model->normal(iface), 0.0)));
        glm::dvec3 l = glm::normalize(glm::dvec3(uniform_M * glm::dvec4(light_dir, 1.0)));
        
        varying_intensity[nthvert] = std::max(0.0, glm::dot(n, l));
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
        return result;
    }

    virtual bool fragment(glm::dvec3 baryCoord, TGAImage& tex_image, TGAImage& nm_image, TGAColor& color) override {

        // albedo
        glm::dvec3 tex_coord = varying_uvCoords[0] * baryCoord[0] + varying_uvCoords[1] * baryCoord[1] + varying_uvCoords[2] * baryCoord[2];
        TGAColor tex_color = tex_image.get((int)(tex_coord[0] * tex_image.get_width()), (int)(tex_coord[1] * tex_image.get_height()));

        // normal
        TGAColor nm_color = nm_image.get((int)(tex_coord[0] * tex_image.get_width()), (int)(tex_coord[1] * tex_image.get_height()));
        glm::dvec3 normal_color = glm::normalize(glm::dvec3(nm_color[0], nm_color[1], nm_color[2]) * 2.0 - 1.0);
        double normal_coeff = glm::max(glm::dot(normal_color, glm::dvec3(0.0, 0.0, 1.0)), 0.0);

        // diffuse color
        

        // final color
        float intensity = (glm::dot(baryCoord, varying_intensity) + ka);
        color = tex_color * normal_coeff * intensity;

        // clamp each color
        for (int i = 0; i < 3; i++) {
            color[i] = std::min<double>(color[i], 255.0);
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

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage textureImage; 
    textureImage.read_tga_file("african_head_diffuse.tga");
    textureImage.flip_vertically();

    TGAImage normalImage;
    normalImage.read_tga_file("african_head_nm_tangent.tga");
    normalImage.flip_vertically();

    // all transformation matrices
    glm::dvec3 cameraEye = glm::normalize(cameraTarget - camera);
    projection(camera.z);
    viewport(static_cast<double>(width) / 8.0, static_cast<double>(height) / 8.0, static_cast<double>(width) * 0.75, static_cast<double>(height) * 0.75, depth);
    lookAt(cameraEye, camera, glm::dvec3(camera.x, -1, camera.z));

    // populate face
    GouraudShader shader;
    shader.uniform_M = Projection_mat * ModelView_mat;
    shader.uniform_invM = glm::inverse(shader.uniform_M);

    for (int i = 0; i < model->nfaces(); i++) {
        glm::dvec3 pts[3];
        for (int j = 0; j < 3; j++) {
            pts[j] = shader.vertex(i, j);
        }
        triangle(pts, shader, image, textureImage, normalImage, zbuffer);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete model;
    delete[] zbuffer;
    return 0;
}
