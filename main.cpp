//// interpolate with x as dependent variable andd y as independent variable
//void interpolate(std::vector<int>& vec, int x0, int y0, int x1, int y1) {
//	if (y0 > y1) {
//		std::swap(x0, x1);
//		std::swap(y0, y1);
//	}
//	int dx = x1 - x0;
//	int dy = y1 - y0;
//    for (int y=y0; y<=y1; y++) {
//		if (dy != 0) {
//			int x = x0 + (dx * (y - y0)) / dy;
//			vec.push_back(x);
//			continue;
//		} else {
//			vec.push_back(x0);
//			continue;
//		}
//    }
//}
//
//bool compareVertices(Vec2i x0, Vec2i x1) {
//	return (x0.y < x1.y);
//}
//
//// horizontal scanline algorithm: https://gabrielgambetta.com/computer-graphics-from-scratch/07-filled-triangles.html
//void triangleSweepLine(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
//
//	// sort the vertices by y coordinate
//	std::vector<Vec2i> vertices;
//	vertices.push_back(t0); // lowest point
//	vertices.push_back(t1); 
//	vertices.push_back(t2); // highest point
//	std::sort(vertices.begin(), vertices.end(), compareVertices);
//
//	// fill the triangle
//	std::vector<int> left_side;
//	std::vector<int> right_side;
//	std::vector<int> p01; // point between t0 and t1
//	std::vector<int> p12; // point between t1 and t2
//	std::vector<int> p02; // point between t0 and t2
//
//
//	// short side
//	interpolate(p01, vertices[0].x, vertices[0].y, vertices[1].x, vertices[1].y);
//	interpolate(p12, vertices[1].x, vertices[1].y, vertices[2].x, vertices[2].y);
//	// remove the redundant point
//	p01.pop_back();
//	// combine p01 and p12
//	p01.insert(p01.end(), p12.begin(), p12.end());
//
//
//	// tall side
//	interpolate(p02, vertices[0].x, vertices[0].y, vertices[2].x, vertices[2].y);
//
//	// determine which side is left and which side is right
//	int middleIdx = p01.size() / 2;
//	if (p01[middleIdx] < p02[middleIdx]) {
//		left_side = p01;
//		right_side = p02;
//	} else {
//		left_side = p02;
//		right_side = p01;
//	}
//
//	// filling in the triangle
//	for (int y = vertices[0].y; y <= vertices[2].y; y++) {
//		for (int x = left_side[y - vertices[0].y]; x <= right_side[y - vertices[0].y]; x++) {
//			image.set(x, y, color);
//		}
//	}
//}


#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;

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

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    //Approach 1: too much float error
    // Vec3f result;
    //Vec3f v0 = B - A;
    //Vec3f v1 = C - A;
    //Vec3f v2 = P - A;
    //float d00 = v0 * v0;
    //float d01 = v0 * v1;
    //float d11 = v1 * v1;
    //float d20 = v2 * v0;
    //float d21 = v2 * v1;
    //float d = d00 * d11 - d01 * d01;
    //result.y = (d11 * d20 - d01 * d21) / d;
    //result.z = (d00 * d21 - d01 * d20) / d;
    //result.x = 1.0f - result.z - result.y;
    //return result;

    Vec3f s[2];
    for (int i = 2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = cross(s[0], s[1]); // a vector (u,v,1) that is orthogonal to (ABx,ACx,PAx) and (ABy,ACy,PAy) at the same time
    if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3f* pts, Vec3f* texture_coords, float* zbuffer, TGAImage& image, TGAImage& tex_image, float intensity) {
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            Vec3f tex_coord = texture_coords[0] * bc_screen[0] + texture_coords[1] * bc_screen[1] + texture_coords[2] * bc_screen[2];
            //std::cout << tex_coord << std::endl;
            TGAColor tex_color = tex_image.get((int)(tex_coord[0] * tex_image.get_width()), (int)(tex_coord[1] * tex_image.get_height()));
            tex_color.r *= intensity;
            tex_color.g *= intensity;
            tex_color.b *= intensity;
            //std::cout << tex_color.r << " " << tex_color.g << " " << tex_color.b << std::endl;
            P.z = 0;
            for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];
            if (zbuffer[int(P.x + P.y * width)] < P.z) {
                zbuffer[int(P.x + P.y * width)] = P.z;
                image.set(P.x, P.y, tex_color);
            }
        }
    }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}

int main(int argc, char** argv) {
    if (2 == argc) {
        model = new Model(argv[1]);
    }
    else {
        model = new Model("obj/african_head.obj");
    }

    float* zbuffer = new float[width * height];
    for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    Vec3f light_dir(0, 0, 1);
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage textureImage; 
    textureImage.read_tga_file("african_head_diffuse.tga");
    textureImage.flip_vertically();

    // populate face
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        std::vector<int> vertex_tex_idx = model->vert_texture_idx(i);
        Vec3f pts[3];
        Vec3f world_coords[3];
        Vec3f texture_coords[3];
        for (int j = 0; j < 3; j++) {
            world_coords[j] = model->vert(face[j]);
            //std::cout << "tex idx: " << vertex_tex_idx[j] << std::endl;
            texture_coords[j] = model->vert_texture(vertex_tex_idx[j]);
            pts[j] = world2screen(world_coords[j]);
        }
        Vec3f normal = cross((world_coords[1] - world_coords[0]), (world_coords[2] - world_coords[0]));
		normal.normalize();
		float intensity = normal * light_dir;
		if (intensity > 0.f) {
            triangle(pts, texture_coords, zbuffer, image, textureImage, intensity);
		}
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}
