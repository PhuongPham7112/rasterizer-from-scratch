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
    virtual glm::ivec3 vertex(int iface, int nthvert) {
        
    }

    virtual bool fragment(glm::vec3 baryCoord, TGAColor& color) {

    }
};

glm::dvec3 RasterizedCoords(glm::dvec3 v) {
    glm::dvec3 result;
    glm::dvec4 aug_coords (v.x, v.y, v.z, 1.0);
    glm::dvec3 cameraEye = glm::normalize(cameraTarget - camera);

    projection(camera.z);
    viewport(static_cast<double>(width) / 8.0, static_cast<double>(height) / 8.0, static_cast<double>(width) * 0.75, static_cast<double>(height) * 0.75, depth);
    lookAt(cameraEye, camera, glm::dvec3(camera.x, -1, camera.z));
    glm::dvec4 aug_mat = Viewport_mat * Projection_mat * ModelView_mat * aug_coords;

    // make smoother result
    result.x = int(aug_mat[0] / aug_mat[3]);
    result.y = int(aug_mat[1] / aug_mat[3]);
    result.z = aug_mat[2] / aug_mat[3];

    return result;
}

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

    // populate face
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        std::vector<int> vertex_tex_idx = model->vert_texture_idx(i);
        glm::dvec3 pts[3];
        glm::dvec3 world_coords[3];
        glm::dvec3 texture_coords[3];
        for (int j = 0; j < 3; j++) {
            texture_coords[j] = model->vert_texture(vertex_tex_idx[j]);
            world_coords[j] = model->vert(face[j]);
            pts[j] = RasterizedCoords(world_coords[j]);
        }
        glm::dvec3 normal = glm::normalize(glm::cross(world_coords[1] - world_coords[0], world_coords[2] - world_coords[0]));
        double intensity = glm::dot(normal, light_dir);
		if (intensity > 0.0) {
            triangle(pts, texture_coords, image, textureImage, zbuffer, intensity);
		}
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete model;
    delete[] zbuffer;
    return 0;
}
