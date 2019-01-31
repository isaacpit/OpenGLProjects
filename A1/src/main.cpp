#include <iostream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include "../../glm-stable/glm/glm.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Image.h"
#include "ZBuffer.h"
#include "BoundedBox.h"

using namespace std;
using glm::vec3;


// credit for barycentric coordinates
// https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates

void globalsLogicLoop(MinMax& globals_world, vector<tinyobj::shape_t>& shapes, vector<double>& posBuf, tinyobj::attrib_t& attrib);
void mainLogicLoop(MinMax& globals_world, 
	vector<tinyobj::shape_t>& shapes, 
	vector<double>& posBuf, 
	tinyobj::attrib_t& attrib, 
	MinMax& globals_image, 
	Transformation& tran, 
	shared_ptr<Image>& image,
	ZBuffer& zbuf,
	int colorMode);
Transformation worldToImage(int w, int h, MinMax g);


int main(int argc, char **argv)
{
	srand (time(NULL));
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

	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {

		globalsLogicLoop(globals_world, shapes, posBuf, attrib);
	}

	// get transformations for world min and max to image min and max
	Transformation tran = worldToImage(width, height, globals_world);
	MinMax globals_image;
	globals_image.min = vec3(tran.scale * globals_world.min[X_POS] + tran.tx, tran.scale * globals_world.min[Y_POS] + tran.ty, -2016 );
	globals_image.max = vec3(tran.scale * globals_world.max[X_POS] + tran.tx, tran.scale * globals_world.max[Y_POS] + tran.ty, -2016 );
	
	// initialize
	ZBuffer zbuf(width, height);

	mainLogicLoop(globals_world, shapes, posBuf, attrib,globals_image, tran, image, zbuf, colorMode);
	
	
	image->writeToFile(outputFName);
	return 0;
}

void mainLogicLoop(MinMax& globals_world, 
vector<tinyobj::shape_t>& shapes, 
vector<double>& posBuf, 
tinyobj::attrib_t& attrib, 
MinMax& globals_image, 
Transformation& tran, 
shared_ptr<Image>& image,
ZBuffer& zbuf,
int colorMode = 0) {
	for (size_t s = 0; s < shapes.size(); ++s) {

		size_t index_offset = 0;

		for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			vector<vec3> vertices;
			
			size_t fv = shapes[s].mesh.num_face_vertices[f];

			// get 3 vertices for a face
			for(size_t v = 0; v < fv; v++) { 
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
			
				
				vec3 temp_v=vec3(attrib.vertices[3*idx.vertex_index+0], 
				attrib.vertices[3*idx.vertex_index+1],
				attrib.vertices[3*idx.vertex_index+2]);

				vertices.push_back(temp_v);
			}	
			// use vertices of face to create a bounded box
			BoundedBox bb(vertices, globals_world, globals_image, tran,image, &zbuf, colorMode);

			index_offset += fv;
		}
	}
}

void globalsLogicLoop(MinMax& globals_world, vector<tinyobj::shape_t>& shapes, vector<double>& posBuf, tinyobj::attrib_t& attrib) {
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

Transformation worldToImage(int width, int height, MinMax g) {

	double min_world_x = g.min[X_POS];
	double max_world_x = g.max[X_POS];
	double min_world_y = g.min[Y_POS];
	double max_world_y = g.max[Y_POS];
	double sx = abs(double(width) / (max_world_x - min_world_x));
	double sy = abs(double(height) / (max_world_y - min_world_y));

	double scale = (sx < sy) ? sx : sy;

	vector<double> middle_image(2);
	middle_image.at(X_POS) = width / 2;
	middle_image.at(Y_POS) = height / 2; 

	vector<double> middle_world(2);
	// cout << "max_world_x: " << max_world_x << " min_world_x: " << min_world_x << endl;
	middle_world.at(X_POS) = scale * (max_world_x + min_world_x) / 2;
	middle_world.at(Y_POS) = scale * (max_world_y + min_world_y) / 2;

	double tx = middle_image[X_POS] - middle_world[X_POS];
	double ty = middle_image[Y_POS] - middle_world[Y_POS];

	Transformation t;
	t.tx = tx;
	t.ty = ty;
	t.scale = scale;

	return t;

}



