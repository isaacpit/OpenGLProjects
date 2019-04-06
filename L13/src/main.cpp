#include <iostream>
#include <vector>
#include <cmath>
#include <map>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

using namespace std;
using std::sqrt;
using std::map;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> prog;

bool keyToggles[256] = {false}; // only for English keyboards!
glm::vec2 cameraRotations(0, 0);
glm::vec2 mousePrev(-1, -1);

// Control points
vector<glm::vec3> cps;

enum SplineType
{
	CATMULL_ROM = 0,
	BASIS,
	SPLINE_TYPE_COUNT
};

enum ArcLengthMode {
	GAUSSIAN_QUAD = 0,
	LERP,
	ARC_MODE_COUNT
};

enum SearchMode {
	LINEAR = 0,
	BINARY,
	SEARCH_MODE_COUNT
};

SplineType type = CATMULL_ROM;
ArcLengthMode arc_length_mode = GAUSSIAN_QUAD;
SearchMode search_mode = LINEAR;

map<SearchMode, string> search_names;

glm::mat4 Bcr, Bb;

vector<pair<float,float> > usTable;
vector<pair<float, float> > XW_GAUSS;

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
	
	if(action == GLFW_PRESS) {
		switch(key) {
			case GLFW_KEY_S:
				type = (SplineType)((type + 1) % SPLINE_TYPE_COUNT);
				buildTable();
				break;
			case GLFW_KEY_G:
				arc_length_mode = (ArcLengthMode)((arc_length_mode+1) % ARC_MODE_COUNT);
				buildTable();
				break;
			case GLFW_KEY_C:
				cps.clear();
				buildTable();
				break;
			case GLFW_KEY_B:
				search_mode = (SearchMode)((search_mode +1) % SEARCH_MODE_COUNT);
				cout << "search_mode: " << search_mode << endl;
				break;
			case GLFW_KEY_R:
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
					cps.push_back(cp);
				}
				buildTable();
				break;
		}
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];
}

static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	if(mousePrev.x >= 0) {
		glm::vec2 mouseCurr(xmouse, ymouse);
		cameraRotations += 0.01f * (mouseCurr - mousePrev);
		mousePrev = mouseCurr;
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
		if(mods & GLFW_MOD_SHIFT) {
			// Insert a new control point
			// Convert from window coord to world coord assuming that we're
			// using an orthgraphic projection from -1 to 1.
			float aspect = (float)width/height;
			glm::vec4 x;
			x[0] = 2.0f * ((xmouse / width) - 0.5f)* aspect;
			x[1] = 2.0f * (((height - ymouse) / height) - 0.5f);
			x[2] = 0.0f;
			x[3] = 1.0f;
			// Build the current modelview matrix.
			auto MV = make_shared<MatrixStack>();
			MV->rotate(cameraRotations[1], glm::vec3(1, 0, 0));
			MV->rotate(cameraRotations[0], glm::vec3(0, 1, 0));
			// Since the modelview matrix transforms from world to eye coords,
			// we want to invert to go from eye to world.
			x = glm::inverse(MV->topMatrix()) * x;
			cps.push_back(glm::vec3(x));
			buildTable();
		} else {
			mousePrev.x = xmouse;
			mousePrev.y = ymouse;
		}
	} else {
		mousePrev[0] = -1;
		mousePrev[1] = -1;
	}
}

void printTable(vector<pair<float, float> > t) {
	for (int i = 0; i < t.size(); ++i) {
		printf("% 6.3f %10.5f \n", t[i].first, t[i].second);	
	}
	
}

static void init()
{
	GLSL::checkVersion();
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	
	// Initialize the GLSL program.
	prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	prog->init();
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->setVerbose(false);
	
	// Initialize time.
	glfwSetTime(0.0);
	
	keyToggles[(unsigned)'l'] = true;
	
	Bcr[0] = glm::vec4( 0.0f,  2.0f,  0.0f,  0.0f);
	Bcr[1] = glm::vec4(-1.0f,  0.0f,  1.0f,  0.0f);
	Bcr[2] = glm::vec4( 2.0f, -5.0f,  4.0f, -1.0f);
	Bcr[3] = glm::vec4(-1.0f,  3.0f, -3.0f,  1.0f);
	Bcr *= 0.5;
	
	Bb[0] = glm::vec4( 1.0f,  4.0f,  1.0f,  0.0f);
	Bb[1] = glm::vec4(-3.0f,  0.0f,  3.0f,  0.0f);
	Bb[2] = glm::vec4( 3.0f, -6.0f,  3.0f,  0.0f);
	Bb[3] = glm::vec4(-1.0f,  3.0f, -3.0f,  1.0f);
	Bb /= 6.0f;

	XW_GAUSS.push_back(make_pair(-sqrt(3.0f/5.0f), (5.0f / 9.0f)));
	XW_GAUSS.push_back(make_pair(0.0f, (8.0f / 9.0f)));
	XW_GAUSS.push_back(make_pair(sqrt(3.0f/5.0f), (5.0f/9.0f)));

	search_names[LINEAR] = "linear";
	search_names[BINARY] = "binary";

	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);
}

void buildTable()
{
	// INSERT CODE HERE
	usTable.clear();

	// float du = 0.2f;

	// glm::mat4 Gk;
	// for (float u = 0.0f; u < 5.0f; u+=du) {
	// 	int i = u / du;
	// 	int j = i + 1;

	// 	if (keyToggles[(unsigned)'d']) {
	// 		cout << "i: " << i << " j: " << j << endl;
	// 	}
	// }
	printf("Control Points\n");
	for (int i = 0 ; i < cps.size(); ++i) {
		printf("i: %d (%6.3f, %6.3f, %6.3f)\n", i, cps[i].x, cps[i].y, cps[i].z);
	}
	printf("Gaussian Node\n");
	for (int i = 0; i < XW_GAUSS.size(); ++i) {
		printf("i: %d x: %6.3f w: %6.3f \n", i, XW_GAUSS[i].first, XW_GAUSS[i].second);
	}

	int n = 6;
	float du = 1.0f / (float) (n-1);

	float u = 0.0f;
	float s = 0.0f;
	usTable.push_back(make_pair(u, s));


	if (arc_length_mode == GAUSSIAN_QUAD) {
		cout << "Using Gaussian Quadratures to build table..." << endl;
	}
	else {
		cout << "Using Approximation to build table..." << endl;
	}

	glm::mat4 B = (type == CATMULL_ROM ? Bcr : Bb);
	for(int k = 0; k < cps.size() - 3; ++k) {
		glm::mat4 Gk;
		for(int i = 0; i < 4; ++i) {
			Gk[i] = glm::vec4(cps[k+i], 0.0f);
		}
		
		if (keyToggles[(unsigned)'d']) {
			// cout << "k: " << k << endl;
		}

		if (arc_length_mode == GAUSSIAN_QUAD) {
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

				usTable.push_back(make_pair(u_k, s));
				

				// if (keyToggles[(unsigned)'d']) {
				// 	printf("i_a: % 6d u: % 6.3f du: % 6.3f u_k: % 6.3f ", i_a, u, du,u_k);
				// 	printf("p: (% 10.6f, % 10.6f, % 10.6f) ", P_1.x, P_1.y, P_1.z);
				// 	// printf("p_a: (% 10.6f, % 10.6f, % 10.6f) ", P_a.x, P_a.y, P_a.z);
				// 	printf("s: % 10.6f \n", s);
				// }
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

				usTable.push_back(make_pair(u_k, s));

			}
		}

	}

	if (keyToggles[(unsigned)'d']) {
		cout << "t.size: " << usTable.size() << endl;
		printTable(usTable);
	}


	if (keyToggles[(unsigned)'d']) keyToggles[(unsigned)'d'] = false;
}

float s2u(float s)
{
	// cout << "s: " << s << endl;
	if (s <= usTable.front().second) {
		// cout << "edge case!" << endl;
		return usTable.front().first;
	}
	// INSERT CODE HERE
	if (keyToggles[(unsigned)'d']) {
		cout << "Using " << search_names[search_mode] << " search" << endl;
	}

	float res = 0.0f;

	if (search_mode == LINEAR) {
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
	else {
		// binary search
		if (keyToggles[(unsigned)'d']) {
			cout << "TODO" << endl;
		}

	}

	
	if (keyToggles[(unsigned)'d']) keyToggles[(unsigned)'d'] = false;
	return res;
}

void render()
{
	// Update time.
	//double t = glfwGetTime();
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	P->pushMatrix();
	MV->pushMatrix();
	
	double aspect = (double)width/height;
	P->multMatrix(glm::ortho(-aspect, aspect, -1.0, 1.0, -2.0, 2.0));
	MV->rotate(cameraRotations.y, glm::vec3(1, 0, 0));
	MV->rotate(cameraRotations.x, glm::vec3(0, 1, 0));
	
	// Bind the program
	prog->bind();
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	
	// Draw control points
	int ncps = (int)cps.size();
	glPointSize(5.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	for(int i = 0; i < ncps; ++i) {
		glVertex3f(cps[i].x, cps[i].y, cps[i].z);
	}
	glEnd();

	glLineWidth(1.0f);
	if(keyToggles[(unsigned)'l']) {
		glColor3f(1.0f, 0.5f, 0.5f);
		glBegin(GL_LINE_STRIP);
		for(int i = 0; i < ncps; ++i) {
			glVertex3f(cps[i].x, cps[i].y, cps[i].z);
		}
		glEnd();
	}
	
	if(ncps >= 4) {
		// Draw spline
		glm::mat4 B = (type == CATMULL_ROM ? Bcr : Bb);
		glLineWidth(3.0f);
		for(int k = 0; k < ncps - 3; ++k) {
			glm::mat4 Gk;
			for(int i = 0; i < 4; ++i) {
				Gk[i] = glm::vec4(cps[k+i], 0.0f);
			}
			int n = 32; // curve discretization
			glBegin(GL_LINE_STRIP);
			if(k % 2 == 0) {
				// Even segment color
				glColor3f(0.0f, 1.0f, 0.0f);
			} else {
				// Odd segment color
				glColor3f(0.0f, 0.0f, 1.0f);
			}
			for(int i = 0; i < n; ++i) {
				// u goes from 0 to 1 within this segment
				float u = i / (n - 1.0f);
				// Compute spline point at u
				glm::vec4 uVec(1.0f, u, u*u, u*u*u);
				glm::vec3 P(Gk * (B * uVec));
				glVertex3fv(&P[0]);
			}
			glEnd();
		}
		
		// Draw equally spaced points on the spline curve
		if(keyToggles[(unsigned)'a'] && !usTable.empty()) {
			float ds = 0.2;
			if (arc_length_mode == GAUSSIAN_QUAD) {
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
					Gk[i] = glm::vec4(cps[k+i], 0.0f);
				}
				glm::vec4 uVec(1.0f, u, u*u, u*u*u);
				glm::vec3 P(Gk * (B * uVec));
				glVertex3fv(&P[0]);
			}
			glEnd();
		}
	}

	// Unbind the program
	prog->unbind();

	// Pop matrix stacks.
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
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
