#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

struct BoundedBox {
	BoundedBox(vector<int> _v_x, vector<int> _v_y) {
		if (_v_x.size() < 1 || _v_y.size() < 1) {
			cerr << "ERROR: vectors are empty from command line args" << endl;
		}
		v_x = _v_x;
		v_y = _v_y;
		minx=v_x[0];	maxx=v_x[0];
		miny=v_y[0]; maxy=v_y[0];
		for (int i = 0; i < v_x.size(); ++i) {
			if (v_x.at(i) < minx) minx = v_x.at(i);
			if (v_x.at(i) > maxx) maxx = v_x.at(i);
		}
		for (int i = 0; i < v_y.size(); ++i) {
			if (v_y.at(i) < miny) miny = v_y.at(i);
			if (v_y.at(i) > maxy) maxy = v_y.at(i);
		}
		cerr << "rm!! minx: " << minx << endl;
		cerr << "rm!! miny: " << miny << endl;
		cerr << "rm!! maxx: " << maxx << endl;
		cerr << "rm!! maxy: " << maxy << endl;
	}

	void drawImage(shared_ptr<Image> &image) {
		cout << "drawing image" << endl;
		cout << "minx: " << minx << endl;
		cout << "miny: " << miny << endl;
		cout << "maxx: " << maxx << endl;
		cout << "maxy: " << maxy << endl;
		
		for (int i = minx; i <= maxx; ++i) {
			for (int j = miny; j <= maxy; ++j) {
				unsigned char r = 0;
				unsigned char g = 0;
				unsigned char b = 0;
				// int chunk = i / 10;
				// cout << "chunk is : " << chunk << endl;
				vector<float> v_baryCoords = barycentricCompute(i, j);
				// float sum = v_baryCoords.at(0) + v_baryCoords.at(1) + v_baryCoords.at(2);
				// cout << "sum: " << sum << endl;
				bool insideAlpha = v_baryCoords.at(0) >= 0 && v_baryCoords.at(0) <= 1;
				bool insideBeta = v_baryCoords.at(1) >= 0 && v_baryCoords.at(1) <= 1;
				bool insideGamma = v_baryCoords.at(2) >= 0 && v_baryCoords.at(2) <= 1;

				cout << "x: " << i << "y: " << j;
				if (insideAlpha && insideBeta && insideGamma) {
					cout << " IS INSIDE" << endl;
					r = v_baryCoords.at(0) * 255;
					g = v_baryCoords.at(1) * 255;
					b = v_baryCoords.at(2) * 255;
				}
				else {
					cout << " IS NOT INSIDE" << endl;
					r = 0;
					g = 0;
					b = 0;
				}
				// if (chunk % 2 == 0) {
				// 	// cout << "entered even case" << endl;
				// 	r = 255;
				// 	g = 0;
				// 	b = 0;
				// }
				// else {
				// 	// cout << "entered odd case" << endl;
				// 	r = 0;
				// 	g = 0;
				// 	b = 255;
				// }
				
				cout << "setting pixels | x: " << i << 
					" y: " << j << " r: " << int(r) << " g: " << int(g) << " b: " << int(b) << endl;
				image->setPixel(i, j, r, g, b);


			}
		}
		if (v_x.size() != v_y.size()) {
			cerr << 
				"ERROR: input vector sizes do not match in BoundedBox.drawImage()" << endl;
		}
		for (int i = 0; i < v_x.size(); ++i) {
			unsigned char r = 0;
			unsigned char g = 255;
			unsigned char b = 0;
			image->setPixel(v_x.at(i), v_y.at(i), r, g ,b);
		}
	}

	float barycentricHelper(int x, int y, int pos_a, int pos_b) {
		float y_a = v_y.at(pos_a);
		float y_b = v_y.at(pos_b);
		float x_a = v_x.at(pos_a);
		float x_b = v_x.at(pos_b);

		float result = (y_a - y_b) * x + (x_b - x_a) * y + (x_a * y_b) - (x_b * y_a);
		
		return result;
	} 
	vector<float> barycentricCompute(int x, int y) {
		// cout << "entered" << endl;
		float x0 = v_x.at(0);
		float y0 = v_y.at(0);
		float x1 = v_x.at(1);
		float y1 = v_y.at(1);
		float x2 = v_x.at(2);
		float y2 = v_y.at(2);
		float alpha = barycentricHelper(x, y, 1, 2) / 
								barycentricHelper(x0, y0, 1, 2);
		float beta = barycentricHelper(x, y, 2, 0) / 
								barycentricHelper(x1, y1, 2, 0);
		float gamma = barycentricHelper(x, y, 0, 1) / 
								barycentricHelper(x2, y2, 0, 1);
		
		cout << "alpha: " << alpha << endl;
		cout << "beta: " << beta << endl;
		cout << "gamma: " << gamma << endl;
		
		vector<float> result = { alpha, beta, gamma };
		return result;
	}


	int minx;
	int miny;
	int maxx;
	int maxy;
	vector<int> v_x;
	vector<int> v_y;
};

int main(int argc, char **argv)
{
	if(argc < 4) {
		cout << "Usage: L01 filename width height" << endl;
		return 0;
	}
	// Output filename
	string filename(argv[1]);
	// Width of image
	int width = atoi(argv[2]);
	// Height of image
	int height = atoi(argv[3]);
	// Create the image. We're using a `shared_ptr`, a C++11 feature.
	int a_x[] = { atoi(argv[4]), atoi(argv[6]), atoi(argv[8]) };
	int a_y[] = { atoi(argv[5]), atoi(argv[7]), atoi(argv[9]) };

	vector<int> v_x(a_x, a_x + sizeof(a_x) / sizeof(int));
	vector<int> v_y(a_y, a_y + sizeof(a_y) / sizeof(int));
	BoundedBox bb(v_x, v_y);

	auto image = make_shared<Image>(width, height);
	// Draw a rectangle
	// for(int y = 10; y < 20; ++y) {
	// 	for(int x = 20; x < 40; ++x) {
	// 		unsigned char r = 255;
	// 		unsigned char g = 0;
	// 		unsigned char b = 0;
	// 		image->setPixel(x, y, r, g, b);
	// 	}
	// }
	// Write image to file
	bb.drawImage(image);
	image->writeToFile(filename);
	return 0;
}

