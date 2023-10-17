#include "tgaimage.h"
#include "model.h"
#include <vector>
#include <bits/stdc++.h> 

void interpolate(std::vector<Vec2i>& vec, int x0, int y0, int x1, int y1);
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);

int main(int argc, char** argv) {
	TGAImage image(500, 500, TGAImage::RGB);

	// Model* model = new Model("obj/african_head.obj");
	// int width = image.get_width();
	// int height = image.get_height();
	// // parsing by obj format: https://en.wikipedia.org/wiki/Wavefront_.obj_file
	// for (int f = 0; f < model->nfaces(); f++) {
	// 	std::vector<int> face_vertices = model->face(f);
	// 	// draw the triangle
	// 	for (int v = 0; v < 3; v++) {
	// 		// starting vertex
	// 		Vec3f v0 = model->vert(face_vertices[v]); 
	// 		// ending vertex
	// 		Vec3f v1 = model->vert(face_vertices[(v+1)%3]); 
	// 		// obj files indexes start from 1
	// 		int x0 = (v0.x + 1.) * width / 2.; 
	// 		int y0 = (v0.y + 1.) * height / 2.; 
	// 		int x1 = (v1.x + 1.) * width / 2.; 
	// 		int y1 = (v1.y + 1.) * height / 2.; 
	// 		line(x0, y0, x1, y1, image, white); 
	// 	}
	// }
	
	// test triangle
	Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
	Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
	Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)}; 
	triangle(t0[0], t0[1], t0[2], image, red); 
	triangle(t1[0], t1[1], t1[2], image, white); 
	triangle(t2[0], t2[1], t2[2], image, green);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
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

// interpolate with x as dependent variable andd y as independent variable
void interpolate(std::vector<Vec2i>& vec, int x0, int y0, int x1, int y1) {
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
            vec.push_back(Vec2i(y, x)); 
        } else { 
            vec.push_back(Vec2i(x, y)); 
        }
        error2 += derror2; // increment error each time x is incremented
        if (error2 > dx) {  // if error is too big, move to the next y
            y += sy; // moving to the next y dependeing on the direction
            error2 -= dx*2; // update the error after adjusting y
        } 
    }
}

bool compareVertices(Vec2i x0, Vec2i x1) {
	return (x0.y < x1.y);
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
	// sort the vertices by y coordinate
	std::vector<Vec2i> vertices;
	vertices.push_back(t0); // lowest point
	vertices.push_back(t1); 
	vertices.push_back(t2); // highest point
	std::sort(vertices.begin(), vertices.end(), compareVertices);

	// wireframe the triangle
    line(t0.x, t0.y, t1.x, t1.y, image, color); 
    line(t1.x, t1.y, t2.x, t2.y, image, color); 
    line(t2.x, t2.y, t0.x, t0.y, image, color); 

	// fill the triangle
	// compute x_left and x_right arrays
	std::vector<Vec2i> left_side;
	std::vector<Vec2i> right_side;
	std::vector<Vec2i> p01; // point between t0 and t1
	std::vector<Vec2i> p12; // point between t1 and t2
	std::vector<Vec2i> p02; // point between t0 and t2

	// short side
	interpolate(p01, t0.x, t0.y, t1.x, t1.y);
	interpolate(p12, t1.x, t1.y, t2.x, t2.y);
	// remove the redundant point
	p01.pop_back();
	// combine p01 and p12
	p01.insert(p01.end(), p12.begin(), p12.end());

	// tall side
	interpolate(p02, t0.x, t0.y, t2.x, t2.y);

	// determine which side is left and which side is right
	int middleIdx = p01.size() / 2;
	if (p01[middleIdx].x < p02[middleIdx].x) {
		left_side = p01;
		right_side = p02;
	} else {
		left_side = p02;
		right_side = p01;
	}

	// filling in the triangle
	for (int y = vertices[0].y; y < vertices[2].y; y++) {
		for (int x = left_side[y - vertices[0].y].x; x < right_side[y - vertices[0].y].x; x++) {
			image.set(x, y, color);
		}
	}
}