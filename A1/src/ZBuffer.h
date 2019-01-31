#pragma once

#include <iostream>
#include <string>
#include <vector>

using std::vector;
using std::endl;
using std::cout;

struct ZBuffer {
private:
	float min;
	float max;

public: 
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

		if (_z <= min) {
			min=_z;
		}
		if (_z >= max) {
			max = _z;

		}

		if (_z > z.at(i).at(j)) {

			z.at(i).at(j) = _z;
			
			return true;
		}
		return false;
	}
	ZBuffer(int w, int h) {
		min=-__DBL_MAX__;
		max=__DBL_MAX__;
		vector<vector<double>> tmp(w, vector<double> (h, min));
		z = tmp;
	}
};