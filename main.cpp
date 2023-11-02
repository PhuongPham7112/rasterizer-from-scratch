#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "tgaimage.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;

glm::dvec3 camera(0, 0, 4);
glm::dvec3 light_dir(0, 0, 1);

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



// Bressanham's algorithm: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
	bool steep = false; 
	// steep means the line is closer to y axis than x axis
    if (std::abs(x0-x1)<std::abs(y0-y1)) { 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 
	// make sure the line is from left to right
    if (x0>x1) { 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 
    int dx = x1-x0; 
    int dy = y1-y0; 
	int sy = (y1>y0?1:-1);
	/**
	 * The error variable gives us the distance to the best straight line from our current (x, y) pixel. 
	 * Each time error is greater than one pixel, we increase (or decrease) y by one, and decrease the error by one as well.
	*/
    int derror2 = std::abs(dy)*2; 
    int error2 = 0; 
    int y = y0; 
    for (int x=x0; x<=x1; x++) { 
        if (steep) { 
            image.set(y, x, color); 
        } else { 
            image.set(x, y, color); 
        }
        error2 += derror2; // increment error each time x is incremented
        if (error2 > dx) {  // if error is too big, move to the next y
            y += sy; // moving to the next y dependeing on the direction
            error2 -= dx*2; // update the error after adjusting y
        } 
    }
}

glm::dvec3 barycentric(glm::dvec3 A, glm::dvec3 B, glm::dvec3 C, glm::dvec3 P) {
    glm::dvec3 s[2];

    for (int i = 2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }

    glm::dvec3 u = glm::cross(s[0], s[1]); // a vector (u,v,1) that is orthogonal to (ABx,ACx,PAx) and (ABy,ACy,PAy) at the same time
    if (std::abs(u[2]) < 1e-3) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return glm::dvec3(-1.0, 1.0, 1.0); // in this case generate negative coordinates, it will be thrown away by the rasterizator
    return glm::dvec3(1.0 - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void triangle(glm::dvec3* pts, glm::dvec3* texture_coords, double* zbuffer, TGAImage& image, TGAImage& tex_image, double intensity) {
    glm::dvec2 bboxmin(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    glm::dvec2 bboxmax(std::numeric_limits<double>::min(), std::numeric_limits<double>::min());
    glm::dvec2 clamp(static_cast<double>(image.get_width() - 1), static_cast<double>(image.get_height() - 1));
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0.0, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    glm::dvec3 P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x += 1.0) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y += 1.0) {
            glm::dvec3 bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            // calculate texture color
            glm::dvec3 tex_coord = texture_coords[0] * bc_screen[0] + texture_coords[1] * bc_screen[1] + texture_coords[2] * bc_screen[2];
            TGAColor tex_color = tex_image.get((int)(tex_coord[0] * tex_image.get_width()), (int)(tex_coord[1] * tex_image.get_height()));
            tex_color.r *= static_cast<float>(intensity);
            tex_color.g *= static_cast<float>(intensity);
            tex_color.b *= static_cast<float>(intensity);
            tex_color.a = 255;

            // hidden face removal
            P.z = 0.0;
            for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];
            int idx = int(P.x + P.y * width);
            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;
                image.set(P.x, P.y, tex_color);
            }
        }
    }
}

// About viewport: http://learnwebgl.brown37.net/08_projections/projections_viewport.html
// https://glasnost.itcarlow.ie/~powerk/GeneralGraphicsNotes/projection/viewport_transformation.html
// affine transformation: https://en.wikipedia.org/wiki/Affine_transformation
glm::dmat4 Viewport(double x, double y, double w, double h) {
    glm::dmat4 m = glm::dmat4(1.0);
    m[3][0] = x + w / 2.0;
    m[3][1] = y + h / 2.0;
    m[3][2] = static_cast<double>(depth) / 2.0;

    m[0][0] = w / 2.0;
    m[1][1] = h / 2.0;
    m[2][2] = static_cast<double>(depth) / 2.0;
    return m;
}

glm::dmat4 LookAt(glm::dvec3 eye, glm::dvec3 center, glm::dvec3 up) {
    glm::dvec3 forward = glm::normalize(center - eye);
    glm::dvec3 right = glm::normalize(glm::cross(forward, up));
    up = glm::normalize(glm::cross(forward, right));

    glm::dmat4 M_inv = glm::dmat4(1.0);
    glm::dmat4 Tr = glm::dmat4(1.0);

    for (int i = 0; i < 3; i++) {
        M_inv[i][0] = right[i];
        M_inv[i][1] = up[i];
        M_inv[i][2] = forward[i];
        Tr[3][i] = -eye[i];
    }
    return M_inv * Tr;
}

glm::dvec3 CameraProjectedCoords(glm::dvec3 v) {
    glm::dvec3 result;
    // camera projection
    glm::dvec4 aug_coords (v.x, v.y, v.z, 1.0);

    glm::dmat4 proj_mat (1.0);
    proj_mat[2][3] = -1.0 / (camera.z);

    glm::dmat4 viewport_mat = Viewport(static_cast<double>(width) / 8.0, static_cast<double>(height) / 8.0, static_cast<double>(width) * 0.75, static_cast<double>(height) * 0.75);
    glm::dvec3 cameraTarget = glm::dvec3(0, 0, 0);
    glm::dvec3 cameraEye = glm::normalize(cameraTarget - camera);
    glm::dmat4 modelview_mat = LookAt(cameraEye, camera, glm::dvec3(camera.x, -1, camera.z));
    glm::dvec4 aug_mat = viewport_mat * proj_mat * modelview_mat * aug_coords;

    // make smoother result
    result.x = int(aug_mat[0] / aug_mat[3]);
    result.y = int(aug_mat[1] / aug_mat[3]);
    result.z = aug_mat[2] / aug_mat[3];

    return result;
}

glm::dvec3 world2screen(glm::dvec3 v) {
    return glm::dvec3(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
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
            pts[j] = CameraProjectedCoords(world_coords[j]);
        }
        glm::dvec3 normal = glm::normalize(glm::cross(world_coords[1] - world_coords[0], world_coords[2] - world_coords[0]));
        double intensity = glm::dot(normal, light_dir);
		if (intensity > 0.0) {
            triangle(pts, texture_coords, zbuffer, image, textureImage, intensity);
		}
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    { // dump z-buffer (debugging purposes only)
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                zbimage.set(i, j, TGAColor(zbuffer[(i + j * width)], 1));
            }
        }
        zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        zbimage.write_tga_file("zbuffer.tga");
    }

    delete model;
    delete[] zbuffer;
    return 0;
}
