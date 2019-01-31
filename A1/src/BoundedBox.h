#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "../../glm-stable/glm/glm.hpp"

#include "MinMax.h"
#include "Transformation.h"
#include "ZBuffer.h"
#include "Image.h"

using std::vector;
using std::endl;
using std::cout;
using glm::vec3;
using std::shared_ptr;

const int A_POS = 0;
const int B_POS = 1;
const int C_POS = 2;

const int X_POS = 0;
const int Y_POS = 1;
const int Z_POS = 2;

const int RED = 0;
const int GREEN = 1;
const int BLUE = 2;

// void Barycentric(vec3 p, vec3 a, vec3 b, vec3 c, double& u, double& v, double& w);
void Barycentric(vec3 p, vec3 a, vec3 b, vec3 c, double& u, double& v, double& w) {
		
    vec3 v0 = b - a, v1 = c - a, v2 = p - a;
		
    double d00 = glm::dot(v0, v0);
    double d01 = glm::dot(v0, v1);
    double d11 = glm::dot(v1, v1);
    double d20 = glm::dot(v2, v0);
    double d21 = glm::dot(v2, v1);
    double denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}


struct BoundedBox {
	void ToImageCoords(vec3 &vec_image, const vec3 &vec_world) {

		vec_image[X_POS] = tran.scale * vec_world[X_POS] + tran.tx;
		vec_image[Y_POS] = tran.scale * vec_world[Y_POS] + tran.ty;
		vec_image[Z_POS] = vec_world[Z_POS];
		
	}
	
	void findLocalMinMax() {

		local.min = vec3(vertices[A_POS][X_POS], vertices[A_POS][Y_POS], vertices[A_POS][Z_POS]);
		local.max = vec3(vertices[A_POS][X_POS], vertices[A_POS][Y_POS], vertices[A_POS][Z_POS]);
		for (int i = 0; i < vertices.size(); ++i) {
			
			if (vertices.at(i)[X_POS] < local.min[X_POS]) {
				local.min[X_POS] = vertices.at(i)[X_POS];
			}
			if (vertices.at(i)[Y_POS] < local.min[Y_POS]) {
				local.min[Y_POS] = vertices.at(i)[Y_POS];
			}
			if (vertices.at(i)[Z_POS] < local.min[Z_POS]) {
				local.min[Z_POS] = vertices.at(i)[Z_POS];
			}

			if (vertices.at(i)[X_POS] > local.max[X_POS]) {
				local.max[X_POS] = vertices.at(i)[X_POS];
			}
			if (vertices.at(i)[Y_POS] > local.max[Y_POS]) {
				local.max[Y_POS] = vertices.at(i)[Y_POS];
			}
			if (vertices.at(i)[Z_POS] > local.max[Z_POS]) {
				local.max[Z_POS] = vertices.at(i)[Z_POS];
			}
		}

		vertices_image={vec3(), vec3(), vec3()};
		
		local_image.min = vec3();
		local_image.max = vec3();

		ToImageCoords(vertices_image.at(A_POS), vertices.at(A_POS));
		ToImageCoords(vertices_image.at(B_POS), vertices.at(B_POS));
		ToImageCoords(vertices_image.at(C_POS), vertices.at(C_POS));

		ToImageCoords(local_image.min, local.min);
		ToImageCoords(local_image.max, local.max);

	}

	void BoundsLoop(shared_ptr<Image> &image) {

		for (int i = local_image.min[X_POS]; i <= local_image.max[X_POS]; ++i) {
			for (int j = local_image.min[Y_POS]; j <= local_image.max[Y_POS]; ++j) {
				double u, v, w;
				vec3 p = vec3(i, j, 0);
				Barycentric(p, 
				vertices_image.at(A_POS), 
				vertices_image.at(B_POS), 
				vertices_image.at(C_POS),
				u, v, w);
				bool inTriangle = (0 <= u) && (u <= 1) && 
					(0 <= v) && (v <= 1) && 
					(0 <= w) && (w <= 1);
				if (inTriangle) {
					ColorLogic(i, j, u, v, w, image);
				}				
			}
		}
	}

		void ColorLogic(int x, int y, const double &u, const double &v, const double &w, shared_ptr<Image> &image) {
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;

		double interpolatedZ = u * vertices.at(A_POS)[Z_POS] + 
			v * vertices.at(B_POS)[Z_POS] + 
			w * vertices.at(C_POS)[Z_POS];

		bool replaced = zbuf->checkZ(x, y, interpolatedZ); 

		if (mode == 0 && replaced) {
			r = u * colorA.at(RED) + v * colorB.at(RED) + w * colorC.at(RED);
			g = u * colorA.at(GREEN) + v * colorB.at(GREEN) + w * colorC.at(GREEN);
			b = u * colorA.at(BLUE) + v * colorB.at(BLUE) + w * colorC.at(BLUE);

			image->setPixel(x, y, r, g, b);
		}
		else if (mode == 1 && replaced) {

			double width = (globals_world.max[Z_POS] - globals_world.min[Z_POS]);
			double distanceFromMax = globals_world.max[Z_POS] - interpolatedZ;
			double ratio = distanceFromMax / width;

			double scale = 255.0 * (1 - ratio);
			int int_scale = int(scale);

			r = 0;
			g = 0;
			b = int_scale;

			image->setPixel(x, y, r, g, b);
		}
		else if (mode == 2 && replaced) {
			double distanceTop = sqrt(pow(globals_image.max[Y_POS] - y, 2));
			double distanceBot = sqrt(pow(globals_image.min[Y_POS] - y, 2));
			double deltaY = globals_image.max[Y_POS] - globals_image.min[Y_POS];

			double partTop = (1.0f - distanceTop / deltaY);
			double partBot = (1.0f - distanceBot / deltaY);

			r = partTop * topColor.at(RED) + partBot * botColor.at(RED);
			g = partTop * topColor.at(GREEN) + partBot * botColor.at(GREEN);
			b = partTop * topColor.at(BLUE) + partBot * botColor.at(BLUE);

			image->setPixel(x, y, r, g, b);
		}

	}


	BoundedBox(vector<vec3> v, MinMax g_w,MinMax g_i, Transformation t, shared_ptr<Image> &image, ZBuffer* zb, int m = 0) { 
	// image=im;
	zbuf = zb;
	globals_image = g_i;
	globals_world = g_w;
	// globals_world = 
	vertices = v;
	mode = m;
	tran = t;

	if (mode == 0) {
		colorA= {rand() % 256, rand() %256, rand() %256};
		colorB= {rand() % 256, rand() %256, rand() %256};
		colorC= {rand() % 256, rand() %256, rand() %256};
	} 
	else if (mode == 1) {

	}
	else if (mode == 2) {
		topColor = {255, 0 , 0};
		botColor = {0, 0, 255};
	}

	findLocalMinMax();

	BoundsLoop(image);
	
}
	
	
	ZBuffer* zbuf;
	MinMax globals_image;
	MinMax globals_world;
	MinMax local;
	MinMax local_image;
	vector<vec3> vertices_image;
	vector<vec3> vertices;
	Transformation tran;
	int mode;

	vector<int> colorA;
	vector<int> colorB;
	vector<int> colorC;

	vector<int> topColor;
	vector<int> botColor;
};

