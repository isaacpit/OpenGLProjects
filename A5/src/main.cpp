#include <iostream>
#include <vector>
#include <random>

#define _USE_MATH_DEFINES
#include <cmath>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Camera.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"

using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::mat4;
using std::string;
using std::cerr;
using std::endl;
using std::cout;
using std::make_shared;
using std::vector;
using std::uniform_real_distribution;
using std::default_random_engine;
using std::shared_ptr;
using std::printf;
using glm::quat;


bool keyToggles[256] = {false}; // only for English keyboards!

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from

struct CelMatNode {
	CelMatNode(vec4 v1, vec4 v2, vec4 v3, vec4 v4, vec3 v5) : c1(v1), c2(v2), c3(v3), c4(v4), thresh(v5) {};
	vec4 c1;
	vec4 c2;
	vec4 c3;
	vec4 c4;
	vec3 thresh;

	void printNode() {
		printf("%f, %f, %f, %f\n", c1[0], c1[1], c1[2],c1[3]); 
		printf("%f, %f, %f, %f\n", c2[0], c2[1], c2[2],c2[3]); 
		printf("%f, %f, %f, %f\n", c3[0], c3[1], c3[2],c3[3]); 
		printf("%f, %f, %f, %f\n", c4[0], c4[1], c4[2],c4[3]); 
	}
};

struct CelMat {
	vector<CelMatNode*> cel_mats = {};
	int currCelMatsIndex = 0;

	void addNode(CelMatNode* node) { cel_mats.push_back(node); }

	void nextCelMat() {
		currCelMatsIndex = (currCelMatsIndex + 1 >= cel_mats.size()) ? 0 : currCelMatsIndex + 1;
	}
	void prevCelMat() { 
		currCelMatsIndex = (currCelMatsIndex - 1 < 0) ? cel_mats.size() - 1 : currCelMatsIndex - 1;
	}

	void printAll() {
		for (int i =0; i < cel_mats.size(); ++i) {
			cel_mats.at(i)->printNode();
		}
	}
};

enum SplineType
{
	BEZIER = 0,
	CATMULL_ROM,
	BASIS,
	SPLINE_TYPE_COUNT
};

enum GridMode {
	DRAW_GRID,
	NO_GRID,
	GRID_COUNT
};

enum KeyFramesMode {
	DRAW_KEYS,
	NO_KEYS,
	KEYS_COUNT
};

enum FrenetFramesMode {
	DRAW_FRENET,
	NO_FRENET,
	FRENET_COUNT
};

enum HeliMode {
	DRAW_HELI,
	NO_HELI,
	HELI_COUNT
};

enum QuatMode {
	CALC_QUAT,
	LERP_QUAT,
	QUAT_COUNT
};

enum TwirlMode {
	TWIRL_FIX,
	NO_FIX,
	TWIRL_COUNT
};

enum ArcParamMode {
	APPROX_MODE,
	GAUSS_QUAD_MODE,
	ARC_PARAM_COUNT
};

enum SearchMode {
	LINEAR_MODE,
	BINARY_MODE,
	SEARCH_COUNT
};

enum DrawEqualPoints {
	DRAW_EQUAL_POINTS,
	NO_EQUAL_POINTS,
	EQUAL_POINTS_COUNT
};

enum ConstantSpeedMode {
	GO_CONSTANT_SPEED,
	NON_CONSTANT_SPEED,
	CONSTANT_SPEED_COUNT
};

enum CameraMode {
	MOUSE_CAMERA,
	FOLLOW_HELICOPTER_THIRD,
	FOLLOW_HELICOPTER_FIRST,
	CAMERA_MODE_COUNT
};

enum ShaderMode {
	SHADER_CEL,
	SHADER_SIMPLE,
	SHADER_COUNT
};

GridMode gridMode = DRAW_GRID;
KeyFramesMode keyframesMode = DRAW_KEYS;
FrenetFramesMode frenetframesMode = DRAW_FRENET;
SplineType splineType = CATMULL_ROM;
HeliMode heliMode = DRAW_HELI;
QuatMode quatMode = CALC_QUAT;
TwirlMode twirlFix = TWIRL_FIX;
ArcParamMode arcParamMode = APPROX_MODE;
SearchMode searchMode = LINEAR_MODE;
DrawEqualPoints drawEqualPoints = DRAW_EQUAL_POINTS;
ConstantSpeedMode constantSpeedMode = GO_CONSTANT_SPEED;
CameraMode cameraMode = MOUSE_CAMERA;
ShaderMode shaderMode = SHADER_CEL;

CelMat* celMats = nullptr;

shared_ptr<Program> progNormal;
shared_ptr<Program> progSimple;
shared_ptr<Program> progCel;
shared_ptr<Camera> camera;
shared_ptr<Shape> bunny;
shared_ptr<Shape> heliBody1;
shared_ptr<Shape> heliBody2;
shared_ptr<Shape> heliProp1;
shared_ptr<Shape> heliProp2;

vector<std::pair<vec3, quat> > cps;

default_random_engine generator;

float prop1Speed = 3.0f;
float prop2Speed = 3.0f;
float T_MAX = 8.0f;

mat4 B_BEZIER;
mat4 B_CATMULL_ROM;
mat4 B_BASIS;

vector<std::pair<float,float> > usTable;
vector<std::pair<float, float> > XW_GAUSS;

std::map<SearchMode, string> search_names;

void buildTable();

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_K) {
			keyframesMode = (KeyFramesMode) ((keyframesMode + 1) % KEYS_COUNT);
		}
		else if (key == GLFW_KEY_G) {
			gridMode = (GridMode) ((gridMode + 1) % GRID_COUNT);
		}
		else if (key == GLFW_KEY_F) {
			frenetframesMode = (FrenetFramesMode) ((frenetframesMode + 1) % FRENET_COUNT);
		}
		else if (key == GLFW_KEY_H) {
			heliMode = (HeliMode) ((heliMode + 1) % HELI_COUNT);
		}
		else if (key == GLFW_KEY_Q) {
			quatMode = (QuatMode) ((quatMode + 1) % QUAT_COUNT);
		}
		else if (key == GLFW_KEY_T) {
			twirlFix = (TwirlMode) ((twirlFix + 1) % TWIRL_COUNT);
		}
		else if (key == GLFW_KEY_A) {
			arcParamMode = (ArcParamMode) ((arcParamMode + 1) % ARC_PARAM_COUNT);
			buildTable();
		}
		else if (key == GLFW_KEY_S) {
			searchMode = (SearchMode) ((searchMode + 1) % SEARCH_COUNT);
		}
		else if (key == GLFW_KEY_E) {
			drawEqualPoints = (DrawEqualPoints) ((drawEqualPoints + 1) % EQUAL_POINTS_COUNT);
		}
		else if (key == GLFW_KEY_C) {
			constantSpeedMode = (ConstantSpeedMode) ((constantSpeedMode + 1) % CONSTANT_SPEED_COUNT);
		}
		else if (key == GLFW_KEY_SPACE) {
			cameraMode = (CameraMode) ((cameraMode + 1) % CAMERA_MODE_COUNT);
		}
		else if (key == GLFW_KEY_M) {
			celMats->nextCelMat();
		}
		else if (key == GLFW_KEY_V) {
			shaderMode = (ShaderMode) ((shaderMode + 1) % SHADER_COUNT);
		}
		else if (key == GLFW_KEY_B) {
			// debug build table
			cps.clear();
			int n = 8;
			for(int i = 0; i < n; ++i) {
				float alpha = i / (n - 1.0f);
				float angle = 2.0f * M_PI * alpha;
				float radius = cos(2.0f * angle);
				glm::vec3 cp;
				cp.x = radius * cos(angle);
				cp.y = radius * sin(angle);
				cp.z = (1.0f - alpha)*(-0.5) + alpha*0.5;
				vec3 axis = glm::normalize(vec3(cp.x, cp.y, cp.z));
				// float angle = distributionLarge(generator) * 90.0f / 180.0f * M_PI;
				quat q = glm::angleAxis(angle, axis);
				cps.push_back(std::make_pair(cp, q));
			}
			buildTable();
		}
		else if (key == GLFW_KEY_R) {
			// randomly generate points
			cps.clear();
			int nPoints = 5;
			uniform_real_distribution<float> distributionLarge(-10, 10);
			uniform_real_distribution<float> distributionSmall(0, 10);
			for(int i = 0; i < nPoints; ++i) {
				float x = distributionLarge(generator);
				float y = distributionLarge(generator);
				float z = distributionLarge(generator);

				float px = distributionLarge(generator);
				float py = distributionSmall(generator);
				float pz = distributionLarge(generator);
				vec3 axis = glm::normalize(vec3(x, y, z));
				float angle = distributionLarge(generator) * 90.0f / 180.0f * M_PI;
				quat q = glm::angleAxis(angle, axis);
				vec3 p(px, py, pz);
				
				cps.push_back(std::make_pair(p, q));
			}
			// make closed loop by repeating control points
			if (cps.size() > 3) {
				cps.push_back(cps.at(0));
				cps.push_back(cps.at(1));
				cps.push_back(cps.at(2));
			}

			buildTable();

		}
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];
}

static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		camera->mouseMoved(xmouse, ymouse);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(action == GLFW_PRESS) {
		bool shift = mods & GLFW_MOD_SHIFT;
		bool ctrl  = mods & GLFW_MOD_CONTROL;
		bool alt   = mods & GLFW_MOD_ALT;
		camera->mouseClicked(xmouse, ymouse, shift, ctrl, alt);
	}
}

void printMat4(mat4 m) 
{
	for(int i = 0; i < 4; ++i) {
		for(int j = 0; j < 4; ++j) {
			// mat[j] returns the jth column
			printf("%- 5.2f ", m[j][i]);
		}
		printf("\n");
	}
	printf("\n");
}

void printVec4(vec4 v) {
	for (int i = 0; i < 4; ++i) {
		printf("%- 5.2f ", v[i]);
	}
	printf("\n");
	// cout << "v ( " << v.x << ", " << v.y << ", " << v.z << endl;
	
}

void drawGrid() {
	int xMax = 10, xMin = -10;
	float xMaxf = 10.0f, yMaxf = 10.0f, xMinf = -10.0f, yMinf = -10.0f;
	float height = 0.0f;
	vec3 color = vec3(0.0f, 0.0f, 0.0f);

	glColor3f(color.r, color.g, color.b);
	glBegin(GL_LINES);
	for (int i = xMin; i <= xMax; ++i) {

		glVertex3f(i, height, yMinf);
		glVertex3f(i, height, yMaxf);
		glVertex3f(xMinf, height, i);
		glVertex3f(xMaxf, height, i);

	}
	glEnd();
}

void drawHeli(shared_ptr<Program> prog) {
	heliBody1->draw(prog);
	heliBody2->draw(prog);
	heliProp1->draw(prog);
	heliProp2->draw(prog);
}

void drawHeliKeyFrame(vec3 p, quat q, shared_ptr<MatrixStack> MV, shared_ptr<Program> prog) {
	// p
	mat4 E0 = glm::mat4_cast(glm::normalize(q));
	MV->pushMatrix();
	MV->translate(p);
	MV->multMatrix(E0);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	MV->popMatrix();
	drawHeli(prog);
}

void drawFrenetFrame(vec3 v, shared_ptr<MatrixStack> MV, shared_ptr<MatrixStack> P, float length = 1.0f) {
	// progSimple->bind();
	// glUniformMatrix4fv(progSimple->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	// glUniformMatrix4fv(progSimple->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glLineWidth(5);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(v.x, v.y, v.z);
	glVertex3f(v.x + length, v.y, v.z);
	glColor3f(0, 1, 0);
	glVertex3f(v.x, v.y, v.z);
	glVertex3f(v.x, v.y + length, v.z);
	glColor3f(0, 0, 1);
	glVertex3f(v.x, v.y, v.z);
	glVertex3f(v.x, v.y, v.z + length);
	glEnd();
	glLineWidth(1);

	// progSimple->unbind();
}

void printTable(vector<std::pair<float, float> > t) {
	for (int i = 0; i < t.size(); ++i) {
		printf("% 6.3f %10.5f \n", t[i].first, t[i].second);	
	}
}

void buildTable() {
	usTable.clear();

	int n = 6;
	float du = 1.0f / (float) (n-1);

	float u = 0.0f;
	float s = 0.0f;
	usTable.push_back(std::make_pair(u, s));

	if (arcParamMode == GAUSS_QUAD_MODE) {
		cout << "Using Gaussian Quadratures to build table..." << endl;
	}
	else if (arcParamMode == APPROX_MODE) {
		cout << "Using Approximation to build table..." << endl;
	}
	else {
		cerr << "Something went wrong. An unknown arc length parametrization mode was activated." << endl;
	}

	// allow all types for other splines
	glm::mat4 B = B_CATMULL_ROM;

	for(int k = 0; k < cps.size() - 3; ++k) {
		glm::mat4 Gk;
		for(int i = 0; i < 4; ++i) {
			Gk[i] = glm::vec4(cps[k+i].first, 0.0f);
		}
		
		if (keyToggles[(unsigned)'d']) {
			// cout << "k: " << k << endl;
		}

		if (arcParamMode == GAUSS_QUAD_MODE) {
			for (int i = 1; i < n; ++i) {
				float u_k = i / (n - 1.0f) + k;
				u = i / (n - 1.0f) ; // u_b
				float u_a = u - du;
				// Compute spline point at u
				int i_a = u_k / (1.0f / (float) n);

				float sum = 0;

				float tmp_a1 = (u - u_a) / 2.0f;
				float tmp_b1 = (u + u_a) / 2.0f;

				for (int idx_gauss = 0; idx_gauss < XW_GAUSS.size(); ++idx_gauss) {
					float x_i = XW_GAUSS[idx_gauss].first;
					float y_i = tmp_a1 * x_i + tmp_b1;
					
					glm::vec4 u_1Vec(0.0f , 1.0f, 2.0f * y_i, 3 * y_i * y_i);
					glm::vec3 P_1(Gk * (B * u_1Vec));
				
					float norm_P_1 = sqrt(glm::dot(P_1, P_1));
					sum += XW_GAUSS[idx_gauss].second * norm_P_1;
				}
				sum = tmp_a1 * sum;
				s += sum;

				usTable.push_back(std::make_pair(u_k, s));
				
			}
		}
		else {
			
			for(int i = 1; i < n; ++i) {
				// u goes from 0 to 1 within this segment
				float u_k = i / (n - 1.0f) + k;
				u = i / (n - 1.0f) ; // u_b
				float u_a = u - du;
				// Compute spline point at u
				int i_a = u_k / (1.0f / (float) n);


				glm::vec4 u_bVec(1.0f, u, u*u, u*u*u);
				glm::vec4 u_aVec(1.0f, u_a, u_a * u_a, u_a *u_a * u_a);
				glm::vec3 P_b(Gk * (B * u_bVec));
				glm::vec3 P_a(Gk * (B * u_aVec));

				s += glm::length(P_b - P_a);
				if (keyToggles[(unsigned)'d']) {
					printf("i_a: % 6d u: % 6.3f du: % 6.3f u_k: % 6.3f ", i_a, u, du,u_k);
					printf("p: (% 10.6f, % 10.6f, % 10.6f) ", P_b.x, P_b.y, P_b.z);
					printf("p_a: (% 10.6f, % 10.6f, % 10.6f) ", P_a.x, P_a.y, P_a.z);
					printf("s: % 10.6f \n", s);
				}

				usTable.push_back(std::make_pair(u_k, s));

			}
		}

	}

	keyToggles[(unsigned)'d'] = true;
	if (keyToggles[(unsigned)'d']) {
		cout << "t.size: " << usTable.size() << endl;
		printTable(usTable);
	}

}

bool inRange(float s, int idx) {
	
	int idx_next_elem = ((idx + 1) < usTable.size()) ? idx + 1 : usTable.size() - 1;

	float prev_elem = usTable.at(idx).second;
	float next_elem = usTable.at(idx_next_elem).second;
	
	bool result = false;
	if (s > prev_elem && s < next_elem) result = true;
	return result;
}

int binarySearch(float s, int bottom, int top) {
	int mid = std::floor((top + bottom) / 2);

	if (inRange(s, mid)) {
		return mid;
	}
	else if (s < usTable.at(mid).second) {
		return binarySearch(s, bottom, mid-1);
	}
	else if (s > usTable.at(mid).second) {
		return binarySearch(s, mid + 1, top);
	}
	else {
		return -1;
		cerr << "critical error" << endl;
	}

}

float s2u(float s)
{
	
	if (s <= usTable.front().second) {
		return usTable.front().first;
	}
	if (keyToggles[(unsigned)'d']) {
		cout << "Using " << search_names[searchMode] << " search" << endl;
	}

	float res = 0.0f;

	if (searchMode == LINEAR_MODE) {
		// linear search
		int i = 0;
		while (i < usTable.size()) {
			if (usTable.at(i).second > s) {
				float u_0 = usTable.at(i-1).first;
				float u_1 = usTable.at(i).first;
				float s_0 = usTable.at(i-1).second;
				float s_1 = usTable.at(i).second;
				float alpha = (s - s_0) / (s_1 - s_0);
				float u = (1-alpha) * u_0 + alpha * u_1;
				res = u;

				if (s_0 > s || s_1 < s) {
					cout << "CRITICAL ERROR: s does not fall between s_0 and s_1" << endl;
				}
				break;
			}
			i++;
		}

	}
	else if (searchMode == BINARY_MODE) {
		// binary search
		int pos = binarySearch(s, 0, usTable.size()-1);
		float u_0 = usTable.at(pos).first;
		float u_1 = usTable.at(pos+1).first;
		float s_0 = usTable.at(pos).second;
		float s_1 = usTable.at(pos+1).second;
		float alpha = (s - s_0) / (s_1 - s_0);
		float u = (1-alpha) * u_0 + alpha * u_1;
		res = u;
		

	}
	else {
		cerr << "Error: invalid search mode" << endl;
	}

	if (keyToggles[(unsigned)'d']) keyToggles[(unsigned)'d'] = false;
	return res;
}

CelMat* initCelMats() {
	CelMat* c = new CelMat();

	float f1 = 1.0f;
	float f2 = 0.6f;
	float f3 = 0.4f;
	float f4 = 0.2f;

	vec4 c1_1(1.0f, 0.5f, 0.5f, 1.0f);
	vec4 c1_2(0.6f, 0.3f, 0.3f, 1.0f);
	vec4 c1_3(0.4f, 0.2f, 0.2f, 1.0f);
	vec4 c1_4(0.2f, 0.1f, 0.1f, 1.0f);
	vec3 t1_1(0.95f, 0.70f, 0.45f);

	CelMatNode* node1 = new CelMatNode(c1_1, c1_2, c1_3, c1_4, t1_1);

	vec4 c2_1(f1 / 2.0f, f1, f1 / 2.0f, 1.0f);
	vec4 c2_2(f2 / 2.0f, f2, f2 / 2.0f, 1.0f);
	vec4 c2_3(f3 / 2.0f, f3, f3 / 2.0f, 1.0f);
	vec4 c2_4(f4 / 2.0f, f4, f4 / 2.0f, 1.0f);
	vec3 t2_1(0.85f, 0.5f, 0.20f);
	CelMatNode* node2 = new CelMatNode(c2_1, c2_2, c2_3, c2_4, t2_1);

	vec4 c3_1(f1 / 2.0f, f1 / 2.0f, f1, 1.0f);
	vec4 c3_2(f2 / 2.0f, f2 / 2.0f, f2, 1.0f);
	vec4 c3_3(f3 / 2.0f, f3 / 2.0f, f3, 1.0f);
	vec4 c3_4(f4 / 2.0f, f4 / 2.0f, f4, 1.0f);
	vec3 t3_1(0.7f, 0.55f, 0.2f);
	CelMatNode* node3 = new CelMatNode(c3_1, c3_2, c3_3, c3_4, t3_1);

	c->addNode(node1);
	c->addNode(node2);
	c->addNode(node3);
	
	return c;
}

static void init()
{
	GLSL::checkVersion();
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	
	keyToggles[(unsigned)'c'] = true;
	
	// Super simple shader
	progNormal = make_shared<Program>();
	progNormal->setShaderNames(RESOURCE_DIR + "normal_vert.glsl", RESOURCE_DIR + "normal_frag.glsl");
	progNormal->setVerbose(true);
	progNormal->init();
	progNormal->addUniform("P");
	progNormal->addUniform("MV");
	progNormal->addAttribute("aPos");
	progNormal->addAttribute("aNor");
	progNormal->setVerbose(false);
	
	// For drawing the frames
	progSimple = make_shared<Program>();
	progSimple->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	progSimple->setVerbose(true);
	progSimple->init();
	progSimple->addUniform("P");
	progSimple->addUniform("MV");
	progSimple->setVerbose(false);

	// Cel shader
	progCel = make_shared<Program>();
	progCel->setShaderNames(RESOURCE_DIR + "cel_vert.glsl", RESOURCE_DIR + "cel_frag.glsl");
	progCel->setVerbose(true);
	progCel->init();
	progCel->addAttribute("aPos");
	progCel->addAttribute("aNor");
	progCel->addUniform("MV");
	progCel->addUniform("P");
	progCel->addUniform("c1");
	progCel->addUniform("c2");
	progCel->addUniform("c3");
	progCel->addUniform("c4");
	progCel->addUniform("thresh");
	progCel->setVerbose(false);
		
	heliBody1 = make_shared<Shape>();
	heliBody1->loadMesh(RESOURCE_DIR + "helicopter_body1.obj");
	heliBody1->init();

	heliBody2 = make_shared<Shape>();
	heliBody2->loadMesh(RESOURCE_DIR + "helicopter_body2.obj");
	heliBody2->init();
	
	heliProp1 = make_shared<Shape>();
	heliProp1->loadMesh(RESOURCE_DIR + "helicopter_prop1.obj");
	heliProp1->init();

	heliProp2 = make_shared<Shape>();
	heliProp2->loadMesh(RESOURCE_DIR + "helicopter_prop2.obj");
	heliProp2->init();
	
	camera = make_shared<Camera>();

	glm::vec3 p0(1.0f, 0.0f, 0.0f);
	glm::vec3 p1(6.0f, 2.0f, 0.0f);
	glm::vec3 p2(4.0f, 5.0f, -1.0f);
	glm::vec3 p3(-2.0f, -2.0f, 1.0f);
	glm::vec3 p4(0.0f, 4.0f, -1.0f);

	vec3 axis0 = glm::normalize(vec3(1.0f, 1.0f, 0.0f));
	vec3 axis1 = glm::normalize(vec3(-1.0f, 0.0f, 1.0));
	vec3 axis2 = glm::normalize(vec3(1.0f, -1.0f, 0.0f));
	vec3 axis3 = glm::normalize(vec3(0.0f, 1.0f, -1.0f));
	vec3 axis4 = glm::normalize(vec3(1.0f, 1.0f, 1.0f));

	float angle0 = 90.0f / 180.0f * M_PI;
	float angle1 = 90.0f / 180.0f * M_PI;
	float angle2 = 90.0f / 180.0f * M_PI;
	float angle3 = 90.0f / 180.0f * M_PI;
	float angle4 = 90.0f / 180.0f * M_PI;

	quat q0 = glm::angleAxis(angle0, axis0);
	quat q1 = glm::angleAxis(angle1, axis1);
	quat q2 = glm::angleAxis(angle2, axis2);
	quat q3 = glm::angleAxis(angle3, axis3);
	quat q4 = glm::angleAxis(angle4, axis4);

	mat4 E0 = glm::mat4_cast(glm::normalize(q0));
	mat4 E1 = glm::mat4_cast(glm::normalize(q1));
	mat4 E2 = glm::mat4_cast(glm::normalize(q2));
	mat4 E3 = glm::mat4_cast(glm::normalize(q3));
	mat4 E4 = glm::mat4_cast(glm::normalize(q4));

	cps.push_back(std::make_pair(p0, q0));
	cps.push_back(std::make_pair(p1, q1));
	cps.push_back(std::make_pair(p2, q2));
	cps.push_back(std::make_pair(p3, q3));
	cps.push_back(std::make_pair(p4, q4));
	cps.push_back(std::make_pair(p0, q0));
	cps.push_back(std::make_pair(p1, q1));
	cps.push_back(std::make_pair(p2, q2));


	// Gaussian Quadrature constants
	XW_GAUSS.push_back(std::make_pair(-sqrt(3.0f/5.0f), (5.0f / 9.0f)));
	XW_GAUSS.push_back(std::make_pair(0.0f, (8.0f / 9.0f)));
	XW_GAUSS.push_back(std::make_pair(sqrt(3.0f/5.0f), (5.0f/9.0f)));

	// create B matrices for splines
	// Fill column by column
	B_BEZIER[0] = vec4(1.0f, 0.0f, 0.0f, 0.0f);
	B_BEZIER[1] = vec4(-3.0f, 3.0f, 0.0f, 0.0f);
	B_BEZIER[2] = vec4(3.0f, -6.0f, 3.0f, 0.0f);
	B_BEZIER[3] = vec4(-1.0f, 3.0f, -3.0f, 1.0f);


	B_CATMULL_ROM[0] = vec4(0.0f, 2.0f, 0.0f, 0.0f);
	B_CATMULL_ROM[1] = vec4(-1.0f, 0.0f, 1.0f, 0.0f);
	B_CATMULL_ROM[2] = vec4(2.0f, -5.0f, 4.0f, -1.0f);
	B_CATMULL_ROM[3] = vec4(-1.0f, 3.0f, -3.0f, 1.0f);
	B_CATMULL_ROM = 0.5f * B_CATMULL_ROM;


	B_BASIS[0] = vec4(1.0f, 4.0f, 1.0f, 0.0f);
	B_BASIS[1] = vec4(-3.0f, 0.0f, 3.0f, 0.0f);
	B_BASIS[2] = vec4(3.0f, -6.0f, 3.0f, 0.0f);
	B_BASIS[3] = vec4(-1.0f, 3.0f, -3.0f, 1.0f);
	B_BASIS = (1.0f /6.0f) * B_BASIS;

	search_names[SearchMode::BINARY_MODE] = "binary";
	search_names[SearchMode::LINEAR_MODE] = "linear";

	celMats = initCelMats();
	celMats->printAll();

	buildTable();
	
	// Initialize time.
	glfwSetTime(0.0);
	
	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);
}

void render()
{
	// Update time.
	double t = glfwGetTime();
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	
	// Use the window size for camera.
	glfwGetWindowSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'l']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	vec4 p_i; // point used to travel the curve
	vec4 p_1; // first derivative
	vec4 p_2; // second derivative
	
	mat4* B = nullptr;
	if (splineType == BEZIER) {
		B = &B_BEZIER;
	}
	else if (splineType == CATMULL_ROM) {
		B = &B_CATMULL_ROM;
	}
	else if (splineType == BASIS) {
		B = &B_BASIS;
	}
	
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();


	float uMax = cps.size() - 3;

	mat4 G;
	glColor3f(1.0f, 0.0f, 0.5f);

	// calculate helicopters current position 
	float kfloat;
	float speed = 0.5f;


	float smax = usTable.back().second;
	float tNorm = std::fmod(t, T_MAX) / T_MAX;
	float sNorm = tNorm;
	float s_const = smax * sNorm;


	float u0_1 = 0.0f;

	// pick speed mode 
	if (constantSpeedMode == NON_CONSTANT_SPEED) {
		u0_1 = std::modf(std::fmod(t*speed, cps.size()-3.0f), &kfloat);
	}
	else if (constantSpeedMode == GO_CONSTANT_SPEED) {
		u0_1 = std::modf(std::fmod(s2u(s_const), cps.size() -3.0f), &kfloat);
	}
	else {
		cerr << "Error: speed mode not defined" << endl;
	}
	
	
	int k = (int)std::floor(kfloat);

	G[0] = glm::vec4(cps[k].first, 0.0f);
	G[1] = glm::vec4(cps[k+1].first, 0.0f);
	G[2] = glm::vec4(cps[k+2].first, 0.0f);
	G[3] = glm::vec4(cps[k+3].first, 0.0f);

	vec4 uVec0 = vec4(1.0f, u0_1, u0_1*u0_1, u0_1*u0_1*u0_1);
	vec4 uVec1 = vec4(0.0f, 1.0f, 2.0f * u0_1, 3.0f * u0_1 * u0_1); // derivative
	vec4 uVec2 = vec4(0.0f, 0.0f, 2.0f , 6.0f * u0_1 ); // second derivative

	p_i = G* (*B * uVec0);
	p_1 = G *(*B * uVec1); 
	p_2 = G *(*B * uVec2);

	float len = 0.5f;


	mat4 E;
	if (quatMode == CALC_QUAT) {

		vec4 g0 = vec4(cps.at(k).second.x, cps.at(k).second.y, cps.at(k).second.z, cps.at(k).second.w);
		vec4 g1 = vec4(cps.at(k+1).second.x, cps.at(k+1).second.y, cps.at(k+1).second.z, cps.at(k+1).second.w);
		vec4 g2 = vec4(cps.at(k+2).second.x, cps.at(k+2).second.y, cps.at(k+2).second.z, cps.at(k+2).second.w);
		vec4 g3 = vec4(cps.at(k+3).second.x, cps.at(k+3).second.y, cps.at(k+3).second.z, cps.at(k+3).second.w);
		
		G = mat4(g0, g1, g2, g3);
		if (keyToggles['d']) {
			cout << "B4: " << endl;
			printMat4(G);
		}
		
		for (int i = 0; i < 3; ++i) {
			float dot_prod = glm::dot(G[i], G[i+1]) ;
			if (keyToggles['d']) {
				cout << i << " dot: " << dot_prod << endl;;
			}
			if (twirlFix == TWIRL_FIX) {
				if (dot_prod< 0) {
					G[i+1] = - G[i+1];
				}
			}

		}
		if (keyToggles['d']) {
			cout << "AFTER: " << endl;
			printMat4(G);
		}
		vec4 qVec = G*(* B * uVec0);
		quat q_test(qVec[3], qVec[0], qVec[1], qVec[2]);
		E = glm::mat4_cast(glm::normalize(q_test)); // Creates a rotation matrix
		E[3] = glm::vec4(p_i.x, p_i.y, p_i.z, 1.0f); // Puts 'p', which is a vec3 that represents the position, into the last column

	}
	else if (quatMode == LERP_QUAT) {
		int prev_idx = k+1;
		int curr_idx = prev_idx + 1;
		quat q = (1.0f - u0_1) * cps.at(prev_idx).second + u0_1 * cps.at(curr_idx).second;
		E = glm::mat4_cast(glm::normalize(q));
		E[3] = glm::vec4(p_i.x, p_i.y, p_i.z, 1.0f);
		
		if (keyToggles[(unsigned)'d']) {
			printf("//////////////\n");
			printf("k %d | %6.3f %6.3f %6.3f %6.3f\n", prev_idx, q.x, q.y, q.z);
			for (int i = 0; i < cps.size(); ++i) {
				printf("i %d | %6.3f %6.3f %6.3f %6.3f\n", i, cps.at(i).second.x, cps.at(i).second.y, cps.at(i).second.z);
			}
		}
	}
	else {
		cerr << "ERROR: bad quat mode" << endl;
	}

	
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	if (cameraMode == MOUSE_CAMERA) {
		camera->applyViewMatrix(MV);
	}
	else if (cameraMode == FOLLOW_HELICOPTER_THIRD || cameraMode == FOLLOW_HELICOPTER_FIRST) {
		vec3 firstPersonOffset(0.0f, 0.1f, 0.0f);
		vec3 thirdPersonOffset(0.0f, 0.0f, 2.0f);
		MV->pushMatrix();

		MV->multMatrix(E);
		if (cameraMode == FOLLOW_HELICOPTER_THIRD) {
			MV->rotate(90.0f /180.0f * M_PI, vec3(0.0f, 1.0f, 0.0f)); // rotate 90 degrees on y axis to face forward
			MV->translate(thirdPersonOffset);
		}
		else if (cameraMode == FOLLOW_HELICOPTER_FIRST) { 
			MV->rotate(90.0f /180.0f * M_PI, vec3(0.0f, 1.0f, 0.0f)); // rotate 90 degrees on y axis to face forward
			MV->translate(firstPersonOffset);
		}

		mat4 invMat = glm::inverse(MV->topMatrix());
		MV->popMatrix();
		MV->multMatrix(invMat);
	}
	else {
		cerr << "Error: undefined camera control mode." << endl;
	}

	
	// Draw origin frame
	progSimple->bind();
	glUniformMatrix4fv(progSimple->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(progSimple->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

	if (gridMode == DRAW_GRID) {
		drawGrid();
	}
	
	if (frenetframesMode == DRAW_FRENET) {
		for (int i = 0; i < cps.size(); ++i) {
			drawFrenetFrame(cps.at(i).first, MV, P, len);
		}

		if (cps.size() >= 4) {
		// drawing curves between points
			glLineWidth(1.0f);
			for (int i = 0; i < cps.size()-3; ++i){
				glBegin(GL_LINE_STRIP);
				G[0] = glm::vec4(cps[i].first, 0.0f);
				G[1] = glm::vec4(cps[i+1].first, 0.0f);
				G[2] = glm::vec4(cps[i+2].first, 0.0f);
				G[3] = glm::vec4(cps[i+3].first, 0.0f);

				for(float u0 = 0.0f; u0 < 1.0f; u0+=0.01f) {
					// Fill in uVec
					glm::vec4 uVec(1.0f, u0, u0*u0, u0*u0*u0);
					// Compute position at u
					glm::vec4 p = G*(*B*uVec);

					glVertex3f(p.x, p.y, p.z);
				}
				glEnd();
			}
		}

		// draw moving frenet frame
		vec4 cross_p1_p2 = vec4(glm::cross(vec3(p_1), vec3(p_2)), 0.0f);

		vec4 tangent = p_1 / glm::length(p_1); // T(u) = p'(u) / ||p'(u)||
		vec4 binorm = cross_p1_p2 / glm::length(cross_p1_p2); // B(u) = p'(u) x p''(u) / ||p'(u) x p''(u)||
		vec4 normal = vec4(glm::cross(vec3(tangent), vec3(binorm)), 0.0f); // N(u) = T(u) x B(u)

		float magnitudeOfLines = 0.25f;
		glLineWidth(5.0f);

		// tangent
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_STRIP);

		vec4 tan_p0 = p_i + 0.0f * tangent;
		glVertex3f(tan_p0.x, tan_p0.y, tan_p0.z);
		vec4 tan_p1 = p_i + magnitudeOfLines * tangent;
		glVertex3f(tan_p1.x, tan_p1.y, tan_p1.z);
		glEnd();

		// binorm
		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_LINE_STRIP);

		vec4 binorm_p0 = p_i + 0.0f * binorm;
		glVertex3f(binorm_p0.x, binorm_p0.y, binorm_p0.z);
		vec4 binorm_p1 = p_i + magnitudeOfLines * binorm;
		glVertex3f(binorm_p1.x, binorm_p1.y, binorm_p1.z);
		glEnd();

		// normal
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINE_STRIP);

		vec4 norm_p0 = p_i + 0.0f * normal;
		glVertex3f(norm_p0.x, norm_p0.y, norm_p0.z);
		vec4 norm_p1 = p_i + magnitudeOfLines * normal;
		glVertex3f(norm_p1.x, norm_p1.y, norm_p1.z);
		glEnd();


		glLineWidth(1);
	}

	if((drawEqualPoints == DRAW_EQUAL_POINTS) && !usTable.empty()) {
			float ds = 0.2;
			if (arcParamMode == GAUSS_QUAD_MODE) {
				glColor3f(0.0f, 0.0f, 1.0f);
			}
			else {
				glColor3f(1.0f, 0.0f, 0.0f);
			}
			
			glPointSize(10.0f);
			glBegin(GL_POINTS);
			float smax = usTable.back().second; // spline length
			for(float s = 0.0f; s < smax; s += ds) {
				// Convert from s to (concatenated) u
				float uu = s2u(s);
				// Convert from concatenated u to the usual u between 0 and 1.
				float kfloat;
				float u = std::modf(uu, &kfloat);
				// k is the index of the starting control point
				int k = (int)std::floor(kfloat);
				// Compute spline point at u
				glm::mat4 Gk;
				for(int i = 0; i < 4; ++i) {
					Gk[i] = glm::vec4(cps[k+i].first, 0.0f);
				}
				glm::vec4 uVec(1.0f, u, u*u, u*u*u);
				glm::vec3 P(Gk * (*B * uVec));
				glVertex3fv(&P[0]);
			}
			glEnd();
		}




	progSimple->unbind();
	GLSL::checkError(GET_FILE_LINE);
	
	shared_ptr<Program> prog;
	if (shaderMode == SHADER_CEL) {
		prog = progCel;

		prog->bind();

		CelMatNode* currNode = celMats->cel_mats.at(celMats->currCelMatsIndex);

		glUniform4f(prog->getUniform("c1"), currNode->c1[0], currNode->c1[1], currNode->c1[2], currNode->c1[3]);
		glUniform4f(prog->getUniform("c2"), currNode->c2[0], currNode->c2[1], currNode->c2[2], currNode->c2[3]);
		glUniform4f(prog->getUniform("c3"), currNode->c3[0], currNode->c3[1], currNode->c3[2], currNode->c3[3]);
		glUniform4f(prog->getUniform("c4"), currNode->c4[0], currNode->c4[1], currNode->c4[2], currNode->c4[3]);
		glUniform3f(prog->getUniform("thresh"), currNode->thresh[0], currNode->thresh[1], currNode->thresh[2]);

		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		
	}
	else if (shaderMode == SHADER_SIMPLE) {
		prog = progNormal;
		
		prog->bind();

		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	}

	// The center of the bunny is at (-0.2802, 0.932, 0.0851)
	// glm::vec3 center(-0.2802, 0.932, 0.0851);
	

	
	// The axes of rotatio for the source and target bunnies
	glm::vec3 axis0, axis1;
	axis0.x = keyToggles[(unsigned)'x'] ? 1.0 : 0.0f;
	axis0.y = keyToggles[(unsigned)'y'] ? 1.0 : 0.0f;
	axis0.z = keyToggles[(unsigned)'z'] ? 1.0 : 0.0f;
	axis1.x = keyToggles[(unsigned)'X'] ? 1.0 : 0.0f;
	axis1.y = keyToggles[(unsigned)'Y'] ? 1.0 : 0.0f;
	axis1.z = keyToggles[(unsigned)'Z'] ? 1.0 : 0.0f;

	glm::quat q0, q1;
	if(glm::length(axis0) == 0) {
		q0 = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	} else {
		axis0 = glm::normalize(axis0);
		q0 = glm::angleAxis((float)(90.0f/180.0f*M_PI), axis0);
	}
	if(glm::length(axis1) == 0) {
		q1 = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	} else {
		axis1 = glm::normalize(axis1);
		q1 = glm::angleAxis((float)(90.0f/180.0f*M_PI), axis1);
	}


	vec3 center1 = vec3(0.0, 0.4819, 0.0);
	vec3 center2 = vec3(0.6228f, 0.1179f, 0.1365f);
	

	// Draw the bunny three times: left, right, and interpolated.
	// left:  use p0 for position and q0 for orientation
	// right: use p1 for position and q1 for orientation

	// draw key frames
	if (keyframesMode == DRAW_KEYS) {
		for (int key_idx = 0; key_idx < cps.size()-3; ++key_idx) {
			drawHeliKeyFrame(cps.at(key_idx).first, cps.at(key_idx).second, MV, prog);
		}
	}
	
	if (heliMode == DRAW_HELI) {
		// INTERPOLATED
		MV->pushMatrix();
		
		MV->multMatrix(E);

		MV->pushMatrix();
		MV->rotate(-t * prop1Speed, vec3(0.0, 0.4819, 0.0));
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		// glUniformMatrix4fv(progCel->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		MV->popMatrix();
		// heliProp1->draw(progCel);
		heliProp1->draw(prog);

		MV->pushMatrix();
		MV->translate(center2);
		MV->rotate(t * prop2Speed, vec3(0.0f, 0.0f, 1.0f));
		MV->translate(-center2);

		// glUniformMatrix4fv(progCel->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		MV->popMatrix();
		// heliProp2->draw(progCel);
		heliProp2->draw(prog);


		// glUniformMatrix4fv(progCel->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		MV->popMatrix();

		// heliBody1->draw(progCel);
		// heliBody2->draw(progCel);
		heliBody1->draw(prog);
		heliBody2->draw(prog);

		
	}
	prog->unbind();
	// progCel->unbind();

		// Pop stacks
	MV->popMatrix();
	P->popMatrix();
	GLSL::checkError(GET_FILE_LINE);

	keyToggles[(unsigned)'d'] = false;
}

int main(int argc, char **argv)
{

	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();

	generator.seed(seed1);

	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "YOUR NAME", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
