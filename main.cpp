#include "tgaimage.h"
#include "model.h"
#include <vector>
#include <iostream>
#include <bits/stdc++.h> 

void interpolate(std::vector<int>& vec, int x0, int y0, int x1, int y1);
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);
void triangleSweepLine(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);
void triangleBarycentric(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
Vec3f light_dir(0,0,-1); // define light_dir

int main(int argc, char** argv) {
	TGAImage image(500, 500, TGAImage::RGB);

	Model* model = new Model("obj/african_head.obj");
	int width = image.get_width();
	int height = image.get_height();
	// parsing by obj format: https://en.wikipedia.org/wiki/Wavefront_.obj_file
	for (int f = 0; f < model->nfaces(); f++) {
		std::vector<int> face_vertices = model->face(f);
		Vec2i screen_coords[3]; 
		Vec3f world_coords[3];
		// draw the triangle
		for (int v = 0; v < 3; v++) {
			Vec3f world_v = model->vert(face_vertices[v]); 
			world_coords[v] = world_v;
        	screen_coords[v] = Vec2i((world_v.x+1.)*width/2., (world_v.y+1.)*height/2.); 
		}
		Vec3f normal = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		normal.normalize();
		float intensity = normal * light_dir;
		if (intensity > 0)
			triangleSweepLine(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(255*intensity, 255*intensity, 255*intensity, intensity));
	}

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
void interpolate(std::vector<int>& vec, int x0, int y0, int x1, int y1) {
	if (y0 > y1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
    for (int y=y0; y<=y1; y++) {
		if (dy != 0) {
			int x = x0 + (dx * (y - y0)) / dy;
			vec.push_back(x);
			continue;
		} else {
			vec.push_back(x0);
			continue;
		}
    }
}

bool compareVertices(Vec2i x0, Vec2i x1) {
	return (x0.y < x1.y);
}

// horizontal scanline algorithm: https://gabrielgambetta.com/computer-graphics-from-scratch/07-filled-triangles.html
void triangleSweepLine(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 

	// sort the vertices by y coordinate
	std::vector<Vec2i> vertices;
	vertices.push_back(t0); // lowest point
	vertices.push_back(t1); 
	vertices.push_back(t2); // highest point
	std::sort(vertices.begin(), vertices.end(), compareVertices);

	// fill the triangle
	std::vector<int> left_side;
	std::vector<int> right_side;
	std::vector<int> p01; // point between t0 and t1
	std::vector<int> p12; // point between t1 and t2
	std::vector<int> p02; // point between t0 and t2


	// short side
	interpolate(p01, vertices[0].x, vertices[0].y, vertices[1].x, vertices[1].y);
	interpolate(p12, vertices[1].x, vertices[1].y, vertices[2].x, vertices[2].y);
	// remove the redundant point
	p01.pop_back();
	// combine p01 and p12
	p01.insert(p01.end(), p12.begin(), p12.end());


	// tall side
	interpolate(p02, vertices[0].x, vertices[0].y, vertices[2].x, vertices[2].y);

	// determine which side is left and which side is right
	int middleIdx = p01.size() / 2;
	if (p01[middleIdx] < p02[middleIdx]) {
		left_side = p01;
		right_side = p02;
	} else {
		left_side = p02;
		right_side = p01;
	}

	// filling in the triangle
	for (int y = vertices[0].y; y <= vertices[2].y; y++) {
		for (int x = left_side[y - vertices[0].y]; x <= right_side[y - vertices[0].y]; x++) {
			image.set(x, y, color);
		}
	}
}

void triangleBarycentric(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {

}