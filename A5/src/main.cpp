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


bool keyToggles[256] = {false}; // only for English keyboards!

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from

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

GridMode gridMode = DRAW_GRID;
KeyFramesMode keyframesMode = DRAW_KEYS;
FrenetFramesMode frenetframesMode = DRAW_FRENET;
SplineType splineType = CATMULL_ROM;
HeliMode heliMode = DRAW_HELI;

shared_ptr<Program> progNormal;
shared_ptr<Program> progSimple;
shared_ptr<Camera> camera;
shared_ptr<Shape> bunny;
shared_ptr<Shape> heliBody1;
shared_ptr<Shape> heliBody2;
shared_ptr<Shape> heliProp1;
shared_ptr<Shape> heliProp2;

vector<vec3> keyFramePoints;

float prop1Speed = 3.0f;
float prop2Speed = 3.0f;

mat4 B_BEZIER;
mat4 B_CATMULL_ROM;
mat4 B_BASIS;

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
	int xMax = 10, yMax = 10, xMin = -10, yMin = -10;
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

void drawHeli() {
	heliBody1->draw(progNormal);
	heliBody2->draw(progNormal);
	heliProp1->draw(progNormal);
	heliProp2->draw(progNormal);
}


void drawHeliKeyFrame(vec3 p, shared_ptr<MatrixStack> MV) {
	// p
	MV->pushMatrix();
	MV->translate(p);
	glUniformMatrix4fv(progNormal->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	MV->popMatrix();
	drawHeli();
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

static void init()
{
	GLSL::checkVersion();
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	
	keyToggles[(unsigned)'c'] = true;
	
	// For drawing the bunny
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
	
	bunny = make_shared<Shape>();
	bunny->loadMesh(RESOURCE_DIR + "helicopter_body2.obj");
	bunny->init();
		
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

	glm::vec3 p0_0(-1.0f, 0.0f, 0.0f);
	glm::vec3 p0(-1.0f, 0.0f, 0.0f);
	glm::vec3 p1( 2.0f, 0.0f, 0.0f);
	glm::vec3 p2(4.0f, 3.0f, -1.0f);
	glm::vec3 p3(0.0f, 5.0f, 0.0f);
	glm::vec3 p4(-3.0f, 2.0f, -1.0f);
	glm::vec3 p4_0(-3.0f, 2.0f, -1.0f);

	// keyFramePoints.push_back(p4);
	keyFramePoints.push_back(p0);
	keyFramePoints.push_back(p1);
	keyFramePoints.push_back(p2);
	keyFramePoints.push_back(p3);
	keyFramePoints.push_back(p4);
	// keyFramePoints.push_back(p4_0);
	keyFramePoints.push_back(p0);
	keyFramePoints.push_back(p1);
	keyFramePoints.push_back(p2);
	// keyFramePoints.push_back(p0_0);



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
	
	mat4* B;
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
	
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	camera->applyViewMatrix(MV);


	
	// Draw origin frame
	progSimple->bind();
	glUniformMatrix4fv(progSimple->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(progSimple->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	// glLineWidth(5);
	// glBegin(GL_LINES);
	// glColor3f(1, 0, 0);
	// glVertex3f(0, 0, 0);
	// glVertex3f(1, 0, 0);
	// glColor3f(0, 1, 0);
	// glVertex3f(0, 0, 0);
	// glVertex3f(0, 1, 0);
	// glColor3f(0, 0, 1);
	// glVertex3f(0, 0, 0);
	// glVertex3f(0, 0, 1);
	// glEnd();
	// glLineWidth(1);

	// drawFrenetFrame(vec3(0, 0, 0), MV, P);
	// drawFrenetFrame(vec3(0, 1, 0), MV, P);


	if (gridMode == DRAW_GRID) {
		drawGrid();
	}

	// Alpha is the linear interpolation parameter between 0 and 1
	float alpha = std::fmod(0.5f*t, 1.0f);

	// glm::vec3 p0_0(-1.0f, 0.0f, 0.0f);
	// glm::vec3 p0(-1.0f, 0.0f, 0.0f);
	// glm::vec3 p1( 1.0f, 0.0f, 0.0f);
	// glm::vec3 p2(4.0f, 3.0f, -1.0f);
	// glm::vec3 p3(0.0f, 5.0f, 0.0f);
	// glm::vec3 p4(-3.0f, 2.0f, -1.0f);
	// glm::vec3 p4_0(-3.0f, 2.0f, -1.0f);

	float uMax = keyFramePoints.size() - 3;
	float u = std::fmod(t, uMax);
	float idxF = 0.0f;
	float fracU = std::modf(u, &idxF);
	int idx = (int) idxF;

	// cout << "uMax: " << uMax << " u: " << u << " idxF: " << idxF <<  " idx: " << idx <<  " fracU: " << fracU << endl;

	// glm::vec3 p_i = (1-fracU) * keyFramePoints.at(idx+1) + fracU * keyFramePoints.at(idx+2);
	// cout << "p_i: " << p_i.x << "," << p_i.y << "," <<  p_i.z << endl;


	mat4 G;
	glColor3f(1.0f, 0.0f, 0.5f);

	float kfloat;
	float speed = 0.5f;

	float u0 = std::modf(std::fmod(t*speed, keyFramePoints.size()-3.0f), &kfloat);
	// printf("U: %f\n", u0);
	int k = (int)std::floor(kfloat);

	G[0] = glm::vec4(keyFramePoints[k], 0.0f);
	G[1] = glm::vec4(keyFramePoints[k+1], 0.0f);
	G[2] = glm::vec4(keyFramePoints[k+2], 0.0f);
	G[3] = glm::vec4(keyFramePoints[k+3], 0.0f);

	vec4 uVec0 = vec4(1.0f, u0, u0*u0, u0*u0*u0);
	vec4 uVec1 = vec4(0.0f, 1.0f, 2.0f * u0, 3.0f * u0 * u0); // derivative
	vec4 uVec2 = vec4(0.0f, 0.0f, 2.0f , 6.0f * u0 ); // second derivative

	p_i = G* (*B * uVec0);
	p_1 = G *(*B * uVec1); 
	p_2 = G *(*B * uVec2);

	float len = 0.5f;
	if (frenetframesMode == DRAW_FRENET) {
		for (int i = 0; i < keyFramePoints.size(); ++i) {
			drawFrenetFrame(keyFramePoints.at(i), MV, P, len);
		}
		// drawFrenetFrame(p0, MV, P, len);
		// drawFrenetFrame(p1, MV, P, len);
		// drawFrenetFrame(p2, MV, P, len);
		// drawFrenetFrame(p3, MV, P, len);
		// drawFrenetFrame(p4, MV, P, len);

	


		if (keyToggles[(unsigned)'d']) {
			cout << "keyFramePoints.size(): " << keyFramePoints.size() << endl;

			for (int i = 0; i < keyFramePoints.size(); ++i) {
				printVec4(vec4(keyFramePoints.at(i), 0.0f));
			}
			
		}
		if (keyFramePoints.size() >= 4) {
		// drawing curves between points
			glLineWidth(1.0f);
			for (int i = 0; i < keyFramePoints.size()-3; ++i){
				glBegin(GL_LINE_STRIP);
				G[0] = glm::vec4(keyFramePoints[i], 0.0f);
				G[1] = glm::vec4(keyFramePoints[i+1], 0.0f);
				G[2] = glm::vec4(keyFramePoints[i+2], 0.0f);
				G[3] = glm::vec4(keyFramePoints[i+3], 0.0f);

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
		//////////////////////////////

		vec4 cross_p1_p2 = vec4(glm::cross(vec3(p_1), vec3(p_2)), 0.0f);

		vec4 tangent = p_1 / glm::length(p_1); // T(u) = p'(u) / ||p'(u)||
		vec4 binorm = cross_p1_p2 / glm::length(cross_p1_p2); // B(u) = p'(u) x p''(u) / ||p'(u) x p''(u)||
		vec4 normal = vec4(glm::cross(vec3(tangent), vec3(binorm)), 0.0f); // N(u) = T(u) x B(u)
		
		if (keyToggles[(unsigned) 'd']) {
			cout << "p_i: " << endl;
			printVec4(p_i);
			for (int i = 0; i < keyFramePoints.size(); ++i) {
				printVec4(vec4(keyFramePoints[i], 0.0f));
			}
		}
		

		float magnitudeOfLines = 0.25f;
		glLineWidth(5.0f);

		// tangent
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_STRIP);
		// for (float i = 0; i < magnitudeOfLines; i+=0.01f) {
		// 	vec4 tan_p = p_i + i * tangent;
		// 	glVertex3f(tan_p.x, tan_p.y, tan_p.z);
		// }

		vec4 tan_p0 = p_i + 0.0f * tangent;
		glVertex3f(tan_p0.x, tan_p0.y, tan_p0.z);
		vec4 tan_p1 = p_i + magnitudeOfLines * tangent;
		glVertex3f(tan_p1.x, tan_p1.y, tan_p1.z);
		glEnd();

		// binorm
		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_LINE_STRIP);
		// for (float i = 0; i < magnitudeOfLines; i+=0.01f) {
		// 	vec4 binorm_p = p_i + i * binorm;
		// 	glVertex3f(binorm_p.x, binorm_p.y, binorm_p.z);
		// }

		vec4 binorm_p0 = p_i + 0.0f * binorm;
		glVertex3f(binorm_p0.x, binorm_p0.y, binorm_p0.z);
		vec4 binorm_p1 = p_i + magnitudeOfLines * binorm;
		glVertex3f(binorm_p1.x, binorm_p1.y, binorm_p1.z);
		glEnd();

		// normal
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINE_STRIP);
		// for (float i = 0; i < magnitudeOfLines; i+=0.01f) {
		// 	vec4 norm_p = p_i + i * normal;
		// 	glVertex3f(norm_p.x, norm_p.y, norm_p.z);
		// }
		vec4 norm_p0 = p_i + 0.0f * normal;
		glVertex3f(norm_p0.x, norm_p0.y, norm_p0.z);
		vec4 norm_p1 = p_i + magnitudeOfLines * normal;
		glVertex3f(norm_p1.x, norm_p1.y, norm_p1.z);
		glEnd();

		////////////////////
		glLineWidth(1);
	}

	




	progSimple->unbind();
	GLSL::checkError(GET_FILE_LINE);
	
	// Draw the bunnies
	progNormal->bind();
	// Send projection matrix (same for all bunnies)
	glUniformMatrix4fv(progNormal->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	
	// The center of the bunny is at (-0.2802, 0.932, 0.0851)
	glm::vec3 center(-0.2802, 0.932, 0.0851);
	

	
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
		for (int i = 0; i < keyFramePoints.size(); ++i) {
			drawHeliKeyFrame(keyFramePoints.at(i), MV);
		}
	}
	
	if (heliMode == DRAW_HELI) {
		// INTERPOLATED
		MV->pushMatrix();
		MV->translate(p_i);

		MV->pushMatrix();
		MV->rotate(-t * prop1Speed, vec3(0.0, 0.4819, 0.0));
		glUniformMatrix4fv(progNormal->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		MV->popMatrix();
		heliProp1->draw(progNormal);

		MV->pushMatrix();
		MV->translate(center2);
		MV->rotate(t * prop2Speed, vec3(0.0f, 0.0f, 1.0f));
		MV->translate(-center2);

		glUniformMatrix4fv(progNormal->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		MV->popMatrix();
		heliProp2->draw(progNormal);


		glUniformMatrix4fv(progNormal->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		MV->popMatrix();

		heliBody1->draw(progNormal);
		heliBody2->draw(progNormal);
		// heliProp2->draw(progNormal);
		// drawHeli(MV);

		
	}
	progNormal->unbind();

		// Pop stacks
	MV->popMatrix();
	P->popMatrix();
	GLSL::checkError(GET_FILE_LINE);

	keyToggles[(unsigned)'d'] = false;
}

int main(int argc, char **argv)
{
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
