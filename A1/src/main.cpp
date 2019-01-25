#include <iostream>
#include <string>
#include <iomanip>
#include <cstdlib>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.

using namespace std;

struct ZBuffer {
	vector<vector<float>> z;

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
			z.at(i).at(j) = _z;
			
			return true;
		}
		return false;
	}
	ZBuffer(int w, int h) {
		min=__FLT_MIN__;
		max=__FLT_MIN__;
		vector<vector<float>> tmp(w, vector<float> (h, min));
		z = tmp;

	}
	// void draw

	float min;
	float max;
};

struct Transformation {
	int tx;
	int ty;
	float scale;
	float max_z;
	float min_z;
	float GLOBAL_max_y;
	float GLOBAL_min_y;
};

struct Vertex {
	vector<int> color;
	float x;
	float y;
	float z;

	void printVertex() {
		cout << "x: " << setw(4) << x << " y: " << setw(4) << y 
			<< " z: " << setw(4) << z << endl;
	}

	
};

Vertex operator-(Vertex a, Vertex b) {
	Vertex tmp;
	tmp.x = a.x - b.x;
	tmp.y = a.y - b.y;
	tmp.z = a.z - b.z;
	return tmp;
}

float Dot(Vertex a, Vertex b) {
	float result = 0.0f;
	result = a.x * b.x + a.y * b.y + a.z * b.z;
	return result;



}


struct BoundedBox {
	BoundedBox(Vertex &v1, Vertex &v2, Vertex &v3, int _w, int _h, Transformation _tran) :
			width(_w), height(_h), t(_tran)
	 {
		
		// initialize
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
		min_x = vertices.at(0).x;
		min_y = vertices.at(0).y;
		min_z = vertices.at(0).z;
		max_x = vertices.at(0).x;
		max_y = vertices.at(0).y;
		max_z = vertices.at(0).z;

		for (int i = 0; i < vertices.size(); ++i) {

			// cout << "vertices.at(" << i << ").y: " << vertices.at(i).y << endl;
			// cout << "max_y" << endl;
			if (vertices.at(i).x < min_x) min_x = vertices.at(i).x;
			if (vertices.at(i).y < min_y) min_y = vertices.at(i).y;
			if (vertices.at(i).z < min_z) min_z = vertices.at(i).z;
			if (vertices.at(i).x > max_x) max_x = vertices.at(i).x;
			if (vertices.at(i).y > max_y) max_y = vertices.at(i).y;
			if (vertices.at(i).z > max_z) max_z = vertices.at(i).z;

		}

		// min_x = min_x * t.scale + t.tx;
		// min_y = min_y * t.scale + t.ty;
		// max_x = max_x * t.scale + t.tx;
		// max_y = max_y * t.scale + t.ty;

		// cout << 
		// "BoundedBox::BoundedBox(v1, v2, v3) ----------------------" << endl;
		// cout << "BEFORE *****************" << endl;
		// cout << "x: [" << min_x << ", " << max_x << "]" << endl;
		// cout << "y: [" << min_y << ", " << max_y << "]" << endl;
		// cout << "z: [" << min_z << ", " << max_z << "]" << endl;
		min_x = min_x * t.scale + t.tx;
		min_y = min_y * t.scale + t.ty;
		max_x = max_x * t.scale + t.tx;
		max_y = max_y * t.scale + t.ty;
		// cout << "AFTER *****************" << endl;
		// cout << "x: [" << min_x << ", " << max_x << "]" << endl;
		// cout << "y: [" << min_y << ", " << max_y << "]" << endl;
		// cout << "z: [" << min_z << ", " << max_z << "]" << endl;
		// cout << 
		// "---------------------------------------------------------" << endl;



	}
	BoundedBox(vector<float> &v, int _w, int _h) :
		width(_w), height(_h) 
		{

		min_x = v.at(0);
		min_y = v.at(1);
		min_z = v.at(2);

		max_x = v.at(0);
		max_y = v.at(1);
		max_z = v.at(2);
			// cout << "BEFORE" << endl;
			// cout << "min_x: " << min_x << endl;
			// cout << "min_y: " << min_y << endl;
			// cout << "min_z: " << min_z << endl;
			// cout << "max_x: " << max_x << endl;
			// cout << "max_y: " << max_y << endl;
			// cout << "max_z: " << max_z << endl;


		for (int i = 0; i < v.size(); ++i) {
			// x case
			// cout << "i: " << i << " **********************" << endl;
			// cout << "min_x: " << min_x << endl;
			// cout << "min_y: " << min_y << endl;
			// cout << "min_z: " << min_z << endl;
			// cout << "max_x: " << max_x << endl;
			// cout << "max_y: " << max_y << endl;
			// cout << "max_z: " << max_z << endl;
			if (i % 3 == 0) {
				// cout << "v.at(i): " << v.at(i) << endl;
				// cout << " v.at(i) < min_x: " << (v.at(i) < min_x) << endl;
				// cout << " v.at(i) > max_x: " << (v.at(i) > min_x) << endl;
				if (v.at(i) < min_x) {
					min_x = v.at(i);
				}
				if (v.at(i) > max_x) {
					max_x = v.at(i);
				}
			}
			// y case
			else if (i % 3 == 1) {
				if (v.at(i) < min_y) {
					min_y = v.at(i);
				}
				if (v.at(i) > max_y) {
					max_y = v.at(i);
				}
			}
			// z case 
			else if (i % 3 == 2) {
				if (v.at(i) < min_z) {
					min_z = v.at(i);
				}
				if (v.at(i) > max_z) {
					max_z = v.at(i);
				}
			}
		}


		float sx = abs(float(width) / (max_x - min_x));
		float sy = abs(float(height) / (max_y - min_y));


		// cout << "sx: " << sx << endl;
		// cout << "sy: " << sy << endl;

		Vertex middleOfImage;
		middleOfImage.x = width / 2;
		middleOfImage.y = height / 2;

		// cout << "m(x_i): " << middleOfImage.x << endl;
		// cout << "m(y_i): " << middleOfImage.y << endl;

		scale = (sx < sy) ? sx : sy;
		// cout << "scale: " << scale << endl;

		// cout << "AFTER" << endl;
			cout << "min_x: " << min_x << endl;
			cout << "min_y: " << min_y << endl;
			cout << "min_z: " << min_z << endl;
			cout << "max_x: " << max_x << endl;
			cout << "max_y: " << max_y << endl;
			cout << "max_z: " << max_z << endl;

		
		Vertex middleOfWorld;
		middleOfWorld.x = scale * (max_x + min_x) / 2.0;
		// cout << "max_x - min_x | world: " << scale * (max_x + min_x) << endl;
		middleOfWorld.y = scale * (max_y + min_y) / 2.0;
		// cout << "max_y - min_y | world: " << scale* (max_y + min_y) << endl;

		// cout << "m(x_w): " << middleOfWorld.x << endl;
		// cout << "m(y_w): " << middleOfWorld.y << endl;

		tx = middleOfImage.x - middleOfWorld.x;
		ty = middleOfImage.y - middleOfWorld.y;

		// cout << "tx: " << tx << endl;
		// cout << "ty: " << ty << endl;

		
		t.scale = scale;
		t.tx = tx;
		t.ty = ty;
		t.max_z = max_z;
		t.min_z = min_z;
		t.GLOBAL_max_y = max_y * scale + ty;
		t.GLOBAL_min_y = min_y * scale + ty;

		// cout << 
		// "BoundedBox::BoundedBox(x, y, x2, y2) ----------------------" << endl;
		// cout << "x: [" << min_x << ", " << max_x << "]" << endl;
		// cout << "y: [" << min_y << ", " << max_y << "]" << endl;
		// cout << "z: [" << min_z << ", " << max_z << "]" << endl;
		// cout << 
		// "---------------------------------------------------------" << endl;

	}

	void drawImage(shared_ptr<Image> &image, int mode = 0, ZBuffer *zbuf = new ZBuffer(0, 0)) {

		unsigned char shapeR = 0;
		unsigned char shapeG = 0;
		unsigned char shapeB = 0;

		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;

		// used when color mode = 2
		Vertex top;
		Vertex bot;
		if (mode == 0) {
			// shapeR = rand() % 256;
			// shapeG = rand() % 256;
			// shapeB = rand() % 256;
			// shapeR = rand() % 128 + 127;
			// shapeG = rand() % 128 + 127;
			// shapeB = rand() % 128 + 127;

			unsigned char shapeR0 = rand() % 256;
			unsigned char shapeG0 = rand() % 256;
			unsigned char shapeB0 = rand() % 256;
			unsigned char shapeR1 = rand() % 256;
			unsigned char shapeG1 = rand() % 256;
			unsigned char shapeB1 = rand() % 256;
			unsigned char shapeR2 = rand() % 256;
			unsigned char shapeG2 = rand() % 256;
			unsigned char shapeB2 = rand() % 256;


			vertices.at(0).color = {shapeR0, shapeG0, shapeB0};
			vertices.at(1).color = {shapeR1, shapeG1, shapeB1};
			vertices.at(2).color = {shapeR2, shapeG2, shapeB2};
			
		}
		else if (mode == 1) {
			r = 0;
			g = 0;
			b = 0;
		}
		else if (mode == 2) {

			unsigned char shapeRTop = 255;
			unsigned char shapeGTop = 0;
			unsigned char shapeBTop = 0;
			top.x=0;
			top.y=t.GLOBAL_max_y;
			top.z=0;
			top.color = {shapeRTop, shapeGTop, shapeBTop };


			unsigned char shapeRBot = 0;
			unsigned char shapeGBot = 0;
			unsigned char shapeBBot = 255;
			bot.x=0;
			bot.y=t.GLOBAL_min_y;
			bot.z=0;
			bot.color = { shapeRBot, shapeGBot, shapeBBot};
		}
		// cout << "shapeR: " << int(shapeR) << endl;
		// cout << "shapeG: " << int(shapeG) << endl;
		// cout << "shapeB: " << int(shapeB) << endl;



		// cout << "BEFORE _________ " << endl;
		// cout << "r: " << int(shapeR) << endl;
		// cout << "g: " << int(shapeG) << endl;
		// cout << "b: " << int(shapeB) << endl;

		// cout << "LOOPING THROUGH " << endl;
		// cout << "x: [" << min_x << ", " << max_x << "]" << endl;
		// cout << "y: [" << min_y << ", " << max_y << "]" << endl;
		// cout << "z: [" << min_z << ", " << max_z << "]" << endl;

		int bot_x = min_x; int top_x = max_x; 
		int bot_y = min_y; int top_y = max_y; 
		int bot_z = min_z; int top_z = max_z; 
		// cout << "x: [" << bot_x << ", " << top_x << "]" << endl;
		// cout << "y: [" << bot_y << ", " << top_y << "]" << endl;
		// cout << "z: [" << bot_z << ", " << top_z << "]" << endl;

		for (int i = bot_x; i <= top_x; ++i) {
			for (int j = bot_y; j <= top_y; ++j) {

				// int chunk = i / 10;
				// cout << "chunk is : " << chunk << endl;
				Vertex p;
				// p.x = i;
				// p.y = j;
				
				vector<float> v_baryCoords = barycentricCompute(i, j);
				// if (mode == 0) {
				// 	v_baryCoords = barycentricCompute(i, j);
				// }
				// else if (mode == 2) {
				// 	v_baryCoords = barycentricCompute(i, j);
				// }

				// float sum = v_baryCoords.at(0) + v_baryCoords.at(1) + v_baryCoords.at(2);
				// cout << "sum: " << sum << endl;
				bool insideAlpha = v_baryCoords.at(0) >= 0 && v_baryCoords.at(0) <= 1;
				bool insideBeta = v_baryCoords.at(1) >= 0 && v_baryCoords.at(1) <= 1;
				bool insideGamma = v_baryCoords.at(2) >= 0 && v_baryCoords.at(2) <= 1;

				// cout << "x: " << i << " y: " << j;
				if (insideAlpha && insideBeta && insideGamma) {
					// cout << " IS INSIDE" << endl;
					// cout << "r: " << int(r) << endl;
					// cout << "g: " << int(g) << endl;
					// cout << "b: " << int() << endl;

					// r = shapeR;
					// g = shapeG;
					// b = shapeB;

					if (mode == 0) {
						r = (v_baryCoords.at(0) * vertices.at(0).color.at(0) + 
							v_baryCoords.at(1) * vertices.at(1).color.at(0) + 
							v_baryCoords.at(2) * vertices.at(2).color.at(0) ) ;
						g = (v_baryCoords.at(0) * vertices.at(0).color.at(1) + 
							v_baryCoords.at(1) * vertices.at(1).color.at(1) + 
							v_baryCoords.at(2) * vertices.at(2).color.at(1) ) ;
						b = (v_baryCoords.at(0) * vertices.at(0).color.at(2) + 
							v_baryCoords.at(1) * vertices.at(1).color.at(2) + 
							v_baryCoords.at(2) * vertices.at(2).color.at(2) ) ;

						image->setPixel(i, j, r, g, b);
					}
					else if (mode == 1) {
						// cout << "************************************** " << endl;
						// cout << "v_0.z: " << vertices.at(0).z << endl;
						// cout << "v_1.z: " << vertices.at(1).z << endl;
						// cout << "v_2.z: " << vertices.at(2).z << endl;
						// cout << "v_bary0: " << v_baryCoords.at(0) << endl;
						// cout << "v_bary1: " << v_baryCoords.at(1) << endl;
						// cout << "v_bary2: " << v_baryCoords.at(2) << endl;
						float interpolatedZ = (vertices.at(0).z * v_baryCoords.at(0) + 
							vertices.at(1).z * v_baryCoords.at(1) + 
							vertices.at(2).z * v_baryCoords.at(2));
						// cout << "interpolatedZ: " << interpolatedZ << endl;

						bool replaced = zbuf->checkZ(i, j, interpolatedZ);
						// cout << "was replaced: " << replaced << " : ";

						if (replaced) {
							float diff = (t.max_z - t.min_z) / 2.0f;
							float distance = t.max_z - t.min_z;
							
							// cout << "global.max_z: " << t.max_z << " global.min_z: " << t.min_z << endl;
						
							// if (interpolatedZ >= t.max_z) {
							// 	// cout << "Z IS TOO BIG" << endl;
							// }
							// else {
								// cout << "diff: " << diff << endl;
								// cout << "r before: " << int(r) << endl;
								// if (interpolatedZ > t.max_z)
								if (interpolatedZ > t.max_z) {
									interpolatedZ = t.max_z;
								}
								else if (interpolatedZ < t.min_z) {
									interpolatedZ = t.min_z;
								}

								float lambda = (1.0f - (t.max_z - interpolatedZ)/distance);

								// cout << lambda << endl;
								// r = 225.0f * lambda + 30.0f;
								r = 255.0f * interpolatedZ / diff;
								// r = 225.0f * interpolatedZ / diff + 30.0f;
								// r = 255.0f * (1.0f - (t.max_z - interpolatedZ) / diff);
								// r = 255.0f * (t.max_z - interpolatedZ) 
								// if (r < 50) {
								// 	r = r * 8;
								// }
								// r = 255.0f - r;
								// if (interpolatedZ > t.max_z || interpolatedZ < t.min_z) { 
								// 	cout << "Z IS OUT OF BOUNDS: " << interpolatedZ <<  endl;
								// 	return;
								// }
								g = 0;
								b = 0;
								// cout << "(" << i << ", " << j << ", " << interpolatedZ << ") r: " << int(r) << " g: " << int(g) << " b: " << int(b) << endl;
								image->setPixel(i, j, r, g , b);
							// }

						}
						else {
						// 	r = 0;
						// 	g = 0;
						// 	b = 0;
								// cout << "(" << i << ", " << j << ", " << interpolatedZ << ")" << endl;

						}
						
					}
					else if (mode == 2) {
						float distanceTop = sqrt(pow(top.y - j, 2));
						float distanceBot = sqrt(pow(bot.y - j, 2));
						float deltaY = top.y - bot.y;

						float partTop = (1.0f - distanceTop / deltaY);
						float partBot = (1.0f - distanceBot / deltaY);
						
						cout << "x: " << i << " y: " << j;
						// cout << " distanceTop: " << distanceTop;
						// cout << " distanceBot: " << distanceBot;
						cout << " deltaY: "  << deltaY ; 
						cout << " partTop: " << partTop << " partBot: " << partBot;
						cout << endl;

						r = partTop * top.color.at(0) + partBot * bot.color.at(0);
						g = partTop * top.color.at(1) + partBot * bot.color.at(1);
						b = partTop * top.color.at(2) + partBot * bot.color.at(2);

						image->setPixel(i, j, r, g ,b);
						// r = distanceTop;
						// r = (v_baryCoords.at(0) * vertices.at(0).color.at(0) + 
						// 	v_baryCoords.at(1) * vertices.at(1).color.at(0) + 
						// 	v_baryCoords.at(2) * vertices.at(2).color.at(0) ) ;
						// g = (v_baryCoords.at(0) * vertices.at(0).color.at(1) + 
						// 	v_baryCoords.at(1) * vertices.at(1).color.at(1) + 
						// 	v_baryCoords.at(2) * vertices.at(2).color.at(1) ) ;
						// b = (v_baryCoords.at(0) * vertices.at(0).color.at(2) + 
						// 	v_baryCoords.at(1) * vertices.at(1).color.at(2) + 
						// 	v_baryCoords.at(2) * vertices.at(2).color.at(2) ) ;
					}


					// r = v_baryCoords.at(0) * int(shapeR);
					// g = v_baryCoords.at(1) * int(shapeG);
					// b = v_baryCoords.at(2) * int(shapeB);
					// if (mode == 0) {
					// 	image->setPixel(i, j, r, g, b);
					// }

				}
				// else {
				// 	// cout << " IS NOT INSIDE" << endl;
				// 	r = 0;
				// 	g = 0;
				// 	b = 0;
				// }
				
				// FIXME
				if (mode == 0) {
					// cout << "entered even case" << endl;
					// r = rand() % 256;
					// g = rand() % 256;
					// b = rand() % 256;
				}
				// else {
				// 	// cout << "entered odd case" << endl;
				// 	r = 0;
				// 	g = 0;
				// 	b = 255;
				// }
				
				// cout << "setting pixels | x: " << i << 
				// 	" y: " << j << " r: " << int(r) << " g: " << int(g) << " b: " << int(b) << endl;



			}
		}
	}

	float barycentricHelper(int x, int y, int pos_a, int pos_b) {
		float y_a = vertices.at(pos_a).y * t.scale + t.ty; 
		float y_b = vertices.at(pos_b).y * t.scale + t.ty;
		float x_a = vertices.at(pos_a).x * t.scale + t.tx;
		float x_b = vertices.at(pos_b).x * t.scale + t.tx;
		

		float result = (y_a - y_b) * x + (x_b - x_a) * y + (x_a * y_b) - (x_b * y_a);
		
		return result;
	} 

	vector<float> barycentricCompute(int x, int y) {
		// cout << "entered" << endl;
		float x0 = vertices.at(0).x * t.scale + t.tx;
		float y0 = vertices.at(0).y * t.scale + t.ty;
		float x1 = vertices.at(1).x * t.scale + t.tx;
		float y1 = vertices.at(1).y * t.scale + t.ty;
		float x2 = vertices.at(2).x * t.scale + t.tx;
		float y2 = vertices.at(2).y * t.scale + t.ty;
		float alpha = barycentricHelper(x, y, 1, 2) / 
								barycentricHelper(x0, y0, 1, 2);
		float beta = barycentricHelper(x, y, 2, 0) / 
								barycentricHelper(x1, y1, 2, 0);
		float gamma = barycentricHelper(x, y, 0, 1) / 
								barycentricHelper(x2, y2, 0, 1);

		// int C = 2; int B = 1; int A = 0;
		// Vertex v0 = vertices.at(B) - vertices.at(A);
		// Vertex v1 = vertices.at(C) - vertices.at(B);
		// Vertex v2 = vertices.at()
		
		vector<float> result = { alpha, beta, gamma };



		// bool insideAlpha = result.at(0) >= 0 && result.at(0) <= 1;
		// bool insideBeta = result.at(1) >= 0 && result.at(1) <= 1;
		// bool insideGamma = result.at(2) >= 0 && result.at(2) <= 1;
		// // cout << " (" << setw(5) << x << "," << setw(5) << y << ") => ";
		// // cout << " \u03B1: " << setw(8) <<alpha;
		// // cout << " \u03B2: " << setw(8) << beta;
		// // cout << " \u03B3: " << setw(8) << gamma;
		// if (insideAlpha && insideBeta && insideGamma) {
		// 	// cout << " IN" <<endl;
		// }
		// else {
		// 	// cout << " OUT" << endl;
		// }

		return result;
	}

	Transformation t;

	vector<Vertex> vertices;
	float min_x;
	float min_y;
	float min_z;
	float max_x;
	float max_y;
	float max_z;

	int width;
	int height;

	float scale;
	float tx;
	float ty;
};

int main(int argc, char **argv)
{
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
	vector<float> posBuf; // list of vertex positions
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;
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
				vector<Vertex> vertices;

				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+0]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+1]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+2]);
				}
				// 	Vertex temp_v;
				// 	temp_v.x = attrib.vertices[3*idx.vertex_index+0];
				// 	temp_v.y = attrib.vertices[3*idx.vertex_index+1];
				// 	temp_v.z = attrib.vertices[3*idx.vertex_index+2];

				// 	temp_v.printVertex();
				// 	vertices.push_back(temp_v);
				// }	

				// if (vertices.size() == 3) {
				// 	BoundedBox bb(vertices[0], vertices[1], vertices[2], width, height);

				// 	// bb.drawImage(image);
				// }
				// else {
				// 	cout << 
				// 		"ERROR: face is not a triangle, has more than 3 vertices." << endl;
				// }

				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
				
			}
		}
	}

	BoundedBox bb_parent(posBuf, width, height);
	cout << "got here" << endl;
	ZBuffer zbuf(width, height);
	// cout << "zbuf.w" << zbuf.z[0].size() << endl;
	// for (int i = 0; i < width; ++i) {
	// 	for (int j = 0; j < height; ++j) {
	// 		cout << zbuf.z[i][j] << " ";
	// 	}
	// 	cout << endl;
	// }

	// for (int i = 0; i < posBuf.size(); ++i) {
	// 	// cout << "posBuf[" << i << "]: " << posBuf.at(i) << endl;
	// }

	for (size_t s = 0; s < shapes.size(); ++s) {

		size_t index_offset = 0;

		for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			vector<Vertex> vertices;
			
			size_t fv = shapes[s].mesh.num_face_vertices[f];

			for(size_t v = 0; v < fv; v++) { 
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
			
				Vertex temp_v;
				temp_v.x = attrib.vertices[3*idx.vertex_index+0];
				temp_v.y = attrib.vertices[3*idx.vertex_index+1];
				temp_v.z = attrib.vertices[3*idx.vertex_index+2];

				// temp_v.printVertex();
				vertices.push_back(temp_v);
			}	

				BoundedBox bb(vertices[0], vertices[1], vertices[2], width, height, bb_parent.t);

				bb.drawImage(image, colorMode, &zbuf);
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

	cout << "Number of vertices: " << posBuf.size()/3 << endl;
	
	image->writeToFile(outputFName);
	return 0;
}
