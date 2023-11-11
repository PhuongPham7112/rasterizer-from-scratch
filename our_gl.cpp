#include "our_gl.h"

glm::dmat4 ModelView_mat;
glm::dmat4 Projection_mat;
glm::dmat4 Viewport_mat;

// About viewport: http://learnwebgl.brown37.net/08_projections/projections_viewport.html
// https://glasnost.itcarlow.ie/~powerk/GeneralGraphicsNotes/projection/viewport_transformation.html
// affine transformation: https://en.wikipedia.org/wiki/Affine_transformation
void viewport(double x, double y, double w, double h, double d) {
    Viewport_mat = glm::dmat4(1.0);
    Viewport_mat[3][0] = x + w / 2.0;
    Viewport_mat[3][1] = y + h / 2.0;
    Viewport_mat[3][2] = d / 2.0;

    Viewport_mat[0][0] = w / 2.0;
    Viewport_mat[1][1] = h / 2.0;
    Viewport_mat[2][2] = d / 2.0;
}

void lookAt(glm::dvec3 eye, glm::dvec3 center, glm::dvec3 up) {
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
    ModelView_mat = M_inv * Tr;
}

void projection(double cameraZ) {
    Projection_mat = glm::dmat4(1.0);
    Projection_mat[2][3] = -1.0 / (cameraZ);
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

void triangle(glm::dvec3* pts, IShader& shader, TGAImage& image, TGAImage& tex_image, TGAImage& nm_image, double* zbuffer) {
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
            // check if a point is in the triangle
            glm::dvec3 bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;

            // calculate texture color
            TGAColor tex_color;
            shader.fragment(bc_screen, tex_image, nm_image, tex_color);

            // hidden face removal
            P.z = 0.0;
            for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];
            int idx = int(P.x + P.y * image.get_width());
            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;
                image.set(P.x, P.y, tex_color);
            }
        }
    }
}

// Bressanham's algorithm: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    bool steep = false;
    // steep means the line is closer to y axis than x axis
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    // make sure the line is from left to right
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int sy = (y1 > y0 ? 1 : -1);
    /**
     * The error variable gives us the distance to the best straight line from our current (x, y) pixel.
     * Each time error is greater than one pixel, we increase (or decrease) y by one, and decrease the error by one as well.
    */
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
        error2 += derror2; // increment error each time x is incremented
        if (error2 > dx) {  // if error is too big, move to the next y
            y += sy; // moving to the next y dependeing on the direction
            error2 -= dx * 2; // update the error after adjusting y
        }
    }
}