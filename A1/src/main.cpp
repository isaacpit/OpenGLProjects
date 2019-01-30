#include <iostream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include "../../glm-stable/glm/glm.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Image.h"

const int A_POS = 0;
const int B_POS = 1;
const int C_POS = 2;

const int X_POS = 0;
const int Y_POS = 1;
const int Z_POS = 2;

const int RED = 0;
const int GREEN = 1;
const int BLUE = 2;

// const int R_POS=0;
// const int G_POS=1;
// const int B_POS=2;

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.

using namespace std;
using glm::vec3;

// credit:
// https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates


struct ZBuffer {
	vector<vector<double>> z;

	// returns true if the current z is closer
	bool checkZ(int i, int j, float _z) {
		if (i >= z.size()){
			cout << "ERROR: OUT OF BOUNDS" << endl;
			return false;
		}
		if (j >= z.at(i).size()) {
			cout << "ERROR: OUT OF BOUNDS" << endl;
			return false;
		}

		// cout << "x: " << i << " y: " << j << endl;
		// cout << "currentZ: " << z.at(i).at(j) << endl;
		// cout << "newZ: " << _z << endl;

		if (_z <= min) {
			// cout << "NEW GLOBAL MIN: " << _z << endl;
			// cout << "PREV GLOBAL MIN: " << _z << endl;
			min=_z;
		}
		if (_z >= max) {
			// cout << "NEW GLOBAL MAX: " << _z << endl;
			// cout << "PREV GLOBAL MAX: " << max << endl;
			max = _z;

		}

		if (_z > z.at(i).at(j)) {
			// cout << "currentZ: " << z.at(i).at(j) << endl;
			// cout << "newZ: " << _z << endl;

			z.at(i).at(j) = _z;
			
			return true;
		}
		return false;
	}
	ZBuffer(int w, int h) {
		min=-__DBL_MAX__;
		max=__DBL_MAX__;
		// double BADMIN=__DBL_MIN__;
		vector<vector<double>> tmp(w, vector<double> (h, min));
		z = tmp;

	}
	// void draw

	float min;
	float max;
};


void Barycentric(vec3 p, vec3 a, vec3 b, vec3 c, double& u, double& v, double& w) {
		
    vec3 v0 = b - a, v1 = c - a, v2 = p - a;
		// printf("v0 (%f, %f, %f)\n", v0[0], v0[1], v0[2]);
		
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

struct Transformation {
	double scale;
	int tx;
	int ty;
};


struct MinMax {
	vec3 min;
	vec3 max;
};


struct BoundedBox {


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
			// fixme set colors
			
			// r = 0;
			// g = 255;
			// b = 0;
		}
		else if (mode == 2) {
			topColor = {255, 0 , 0};
			botColor = {0, 0, 255};
			//fixme set colors
		}


		// printf("colorA RGB: {%3i, %3i, %3i}\n", colorA[RED], colorA[GREEN], colorA[BLUE]);
		// printf("colorB RGB: {%3i, %3i, %3i}\n", colorB[RED], colorB[GREEN], colorB[BLUE]);
		// printf("colorC RGB: {%3i, %3i, %3i}\n", colorC[RED], colorC[GREEN], colorC[BLUE]);

		findLocalMinMax();

		BoundsLoop(image);
		
	}
	void findLocalMinMax() {

		local.min = vec3(vertices[A_POS][X_POS], vertices[A_POS][Y_POS], vertices[A_POS][Z_POS]);
		local.max = vec3(vertices[A_POS][X_POS], vertices[A_POS][Y_POS], vertices[A_POS][Z_POS]);
		// printf("A(%f, %f, %f)\n", vertices[A_POS][X_POS], vertices[A_POS][Y_POS], vertices[A_POS][Z_POS]);
		// printf("B(%f, %f, %f)\n", vertices[B_POS][X_POS], vertices[B_POS][Y_POS], vertices[B_POS][Z_POS]);
		// printf("C(%f, %f, %f)\n", vertices[C_POS][X_POS], vertices[C_POS][Y_POS], vertices[C_POS][Z_POS]);
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
		// printf("x in { %10f, %10f, %10f } yields [%10f, %10f] \n", vertices.at(A_POS)[X_POS],
		// 	vertices.at(B_POS)[X_POS],
		// 	vertices.at(C_POS)[X_POS], 
		// 	local.min[X_POS],
		// 	local.max[X_POS]);
		// printf("y in { %10f, %10f, %10f } yields [%10f, %10f] \n", vertices.at(A_POS)[Y_POS],
		// 	vertices.at(B_POS)[Y_POS],
		// 	vertices.at(C_POS)[Y_POS],
		// 	local.min[Y_POS],
		// 	local.max[Y_POS]);
		// printf("z in { %10f, %10f, %10f } yields [%10f, %10f] \n", vertices.at(A_POS)[Z_POS],
		// 	vertices.at(B_POS)[Z_POS],
		// 	vertices.at(C_POS)[Z_POS],
		// 	local.min[Z_POS],
		// 	local.max[Z_POS]);

		// TO IMAGE COORDS
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
		// printf("Loop bounds x[%10f, %10f] y[%10f, %10f]\n", 
		// 	local_image.min[X_POS], 
		// 	local_image.max[X_POS],
		// 	local_image.min[Y_POS], 
		// 	local_image.max[Y_POS]);

		for (int i = local_image.min[X_POS]; i <= local_image.max[X_POS]; ++i) {
			for (int j = local_image.min[Y_POS]; j <= local_image.max[Y_POS]; ++j) {
				double u, v, w;
				vec3 p = vec3(i, j, 0);
				// printf("p (%f, %f, %f)\t", p[0], p[1], p[2]);
				Barycentric(p, 
				vertices_image.at(A_POS), 
				vertices_image.at(B_POS), 
				vertices_image.at(C_POS),
				u, v, w);
				// printf("u+v+w=%f | ", u+v+w);
				// printf("(%10f, %10f, %10f) \n", u, v, w, u+v+w);
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

			// printf("xy: (%3i, %3i) | uvw: {%5f, %5f, %5f} | RGB: (%i, %i, %i)\n", x, y, u, v, w, r, g, b);
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

			// cout << " distanceTop: " << distanceTop << endl;
			// cout << " distanceBot: " << distanceBot << endl;

			double partTop = (1.0f - distanceTop / deltaY);
			double partBot = (1.0f - distanceBot / deltaY);

			r = partTop * topColor.at(RED) + partBot * botColor.at(RED);
			g = partTop * topColor.at(GREEN) + partBot * botColor.at(GREEN);
			b = partTop * topColor.at(BLUE) + partBot * botColor.at(BLUE);

			image->setPixel(x, y, r, g, b);
		}

	}

	void ToImageCoords(vec3 &vec_image, const vec3 &vec_world) {

		vec_image[X_POS] = tran.scale * vec_world[X_POS] + tran.tx;
		vec_image[Y_POS] = tran.scale * vec_world[Y_POS] + tran.ty;
		vec_image[Z_POS] = vec_world[Z_POS];

		// printf("vec_world: (%10f, %10f, %10f) => (%10f, %10f, %10f)\n", 
		// vec_world[X_POS], vec_world[Y_POS], vec_world[Z_POS],
		// vec_image[X_POS], vec_image[Y_POS], vec_image[Z_POS]);
		
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
	// shared_ptr<Image> image;

	vector<int> colorA;
	vector<int> colorB;
	vector<int> colorC;

	vector<int> topColor;
	vector<int> botColor;
};



int main(int argc, char **argv)
{
	srand (time(NULL));
	// const int X_POS=0, Y_POS=1, Z_POS=2;
	if(argc < 2) {
		cout << "Usage: A1 meshfile" << endl;
		return 0;
	}
	srand (time(NULL));
	string meshName(argv[1]);

	string outputFName = argv[2];
	int width = atoi(argv[3]);
	int height = atoi(argv[4]);
	int colorMode = atoi(argv[5]);

	cout << "outputFName: " << outputFName << endl;
	cout << "width: " << width << endl;
	cout << "height: " << height << endl;
	cout << "colorMode: " << colorMode << endl;

	auto image = make_shared<Image>(width, height);

	// Load geometry
	vector<double> posBuf; // list of vertex positions
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;

	MinMax globals_world;
	globals_world.min = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	globals_world.max = vec3(FLT_MIN, FLT_MIN, FLT_MIN);

	vector<vector<double>> frame_buffer;

	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
		// Some OBJ files have different indices for vertex positions, normals,
		// and texture coordinates. For example, a cube corner vertex may have
		// three different normals. Here, we are going to duplicate all such
		// vertices.
		// Loop over shapes
		for(size_t s = 0; s < shapes.size(); s++) {
			// cout << "shape[" << s << "]" << endl;
			// Loop over faces (polygons)
			size_t index_offset = 0;

			for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				// cout << "face[" << f << "]" << endl;
				// vector<Vertex> vertices;

				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+0]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+1]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+2]);
					if (attrib.vertices[3*idx.vertex_index+0] < globals_world.min[0]) {
						globals_world.min[0] = attrib.vertices[3*idx.vertex_index+0];
					}
					if (attrib.vertices[3*idx.vertex_index+1] < globals_world.min[1]) {
						globals_world.min[1] = attrib.vertices[3*idx.vertex_index+1];
					}
					if (attrib.vertices[3*idx.vertex_index+2] < globals_world.min[2]) {
						globals_world.min[2] = attrib.vertices[3*idx.vertex_index+2];
					}
					if (attrib.vertices[3*idx.vertex_index+0] > globals_world.max[0]) {
						globals_world.max[0] = attrib.vertices[3*idx.vertex_index+0];
					}
					if (attrib.vertices[3*idx.vertex_index+1] > globals_world.max[1]) {
						globals_world.max[1] = attrib.vertices[3*idx.vertex_index+1];
					}
					if (attrib.vertices[3*idx.vertex_index+2] > globals_world.max[2]) {
						globals_world.max[2] = attrib.vertices[3*idx.vertex_index+2];
					}
				}

				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
				
			}
		}
	}

	cout << "x: [" << globals_world.min[X_POS] << ", " << globals_world.max[X_POS] << "]" << endl;
	cout << "y: [" << globals_world.min[Y_POS] << ", " << globals_world.max[Y_POS] << "]" << endl;
	cout << "z: [" << globals_world.min[Z_POS] << ", " << globals_world.max[Z_POS] << "]" << endl;
	
	double min_world_x = globals_world.min[X_POS];
	double max_world_x = globals_world.max[X_POS];
	double min_world_y = globals_world.min[Y_POS];
	double max_world_y = globals_world.max[Y_POS];
	double sx = abs(double(width) / (max_world_x - min_world_x));
	double sy = abs(double(height) / (max_world_y - min_world_y));
	cout << "scalex: " << sx << endl;
	cout << "scaley: " << sy << endl;

	double scale = (sx < sy) ? sx : sy;
	cout << "scale: " << scale << endl;

	vector<double> middle_image(2);
	middle_image.at(X_POS) = width / 2;
	middle_image.at(Y_POS) = height / 2; 

	cout << "middle_image X: " << middle_image.at(X_POS) << endl;
	cout << "middle_image Y: " << middle_image.at(Y_POS) << endl;

	vector<double> middle_world(2);
	// cout << "max_world_x: " << max_world_x << " min_world_x: " << min_world_x << endl;
	middle_world.at(X_POS) = scale * (max_world_x + min_world_x) / 2;
	middle_world.at(Y_POS) = scale * (max_world_y + min_world_y) / 2;

	cout << "middle_world X: " << middle_world.at(X_POS) << endl;
	cout << "middle_world Y: " << middle_world.at(Y_POS) << endl;

	double tx = middle_image[X_POS] - middle_world[X_POS];
	double ty = middle_image[Y_POS] - middle_world[Y_POS];

	cout << "tx: " << tx << endl;
	cout << "ty: " << ty << endl;

	Transformation tran;
	tran.tx = tx;
	tran.ty = ty;
	tran.scale = scale;

	MinMax globals_image;

	ZBuffer zbuf(width, height);
	
	globals_image.min = vec3(scale * globals_world.min[X_POS] + tx, scale * globals_world.min[Y_POS] + ty, -2016 );
	globals_image.max = vec3(scale * globals_world.max[X_POS] + tx, scale * globals_world.max[Y_POS] + ty, -2016 );
	vector<int> globals_min_image(2);
	vector<int> globals_max_image(2);
	
	globals_min_image.at(X_POS) = globals_world.min[X_POS] * scale + tx;
	globals_min_image.at(Y_POS) = globals_world.min[Y_POS] * scale + ty;

	globals_max_image.at(X_POS) = globals_world.max[X_POS] * scale + tx;
	globals_max_image.at(Y_POS) = globals_world.max[Y_POS] * scale + ty;

	cout << "image x bounds: [" << globals_min_image[X_POS] << ", " << globals_max_image[X_POS] << "]" << endl;
	cout << "image y bounds: [" << globals_min_image[Y_POS] << ", " << globals_max_image[Y_POS] << "]" << endl;
	// cout << "image y bounds: [" << globals_image.min[Y_POS] << ", " << globals_image.max[Y_POS] << "]" << endl;
	// vector<vector<double>> ZBuffer;

	for (size_t s = 0; s < shapes.size(); ++s) {

		size_t index_offset = 0;

		for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			vector<vec3> vertices;
			
			size_t fv = shapes[s].mesh.num_face_vertices[f];

			for(size_t v = 0; v < fv; v++) { 
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
			
				vec3 temp_v=vec3(attrib.vertices[3*idx.vertex_index+0], attrib.vertices[3*idx.vertex_index+1],
				attrib.vertices[3*idx.vertex_index+2]);
				// printf("%f %f %f\n", temp_v[0], temp_v[1], temp_v[2]);

				// temp_v.printVertex();
				vertices.push_back(temp_v);
			}	

			BoundedBox bb(vertices, globals_world, globals_image, tran,image, &zbuf, colorMode);

				// bb.drawImage(image, colorMode, &zbuf);
				
			// if (vertices.size() == 3) {
			// 	BoundedBox bb(vertices[0], vertices[1], vertices[2], width, height, bb_parent.t);

			// 	bb.drawImage(image);
			// }
			// else {
			// 	cout << 
			// 		"ERROR: face is not a triangle, has more than 3 vertices." << endl;
			// }

			index_offset += fv;
		}
	}

	// cout << "Number of vertices: " << posBuf.size()/3 << endl;
	
	image->writeToFile(outputFName);
	return 0;
}
