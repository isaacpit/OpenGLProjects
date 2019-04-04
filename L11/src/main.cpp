#include <iostream>
#include <vector>
#include <random>
#include <chrono>


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

// using namespace std;
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


GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> prog;

bool keyToggles[256] = {false}; // only for English keyboards!
glm::vec2 cameraRotations(0, 0);
glm::vec2 mousePrev(-1, -1);
default_random_engine generator;

// Control points
vector<glm::vec3> cps;

enum SplineType
{
	BEZIER = 0,
	CATMULL_ROM,
	BASIS,
	SPLINE_TYPE_COUNT
};

SplineType type = BEZIER;

mat4 B_BEZIER;
mat4 B_CATMULL_ROM;
mat4 B_BASIS;
mat4 B_SPLIT_TYPE_COUNT;

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
				break;
			case GLFW_KEY_C:
				cps.clear();
				break;
			case GLFW_KEY_R:
				cps.clear();
				// generator;
				uniform_real_distribution<float> distribution(-0.8, 0.8);
				for(int i = 0; i < 10; ++i) {
					float x = distribution(generator);
					float y = distribution(generator);
					float z = distribution(generator);
					cps.push_back(glm::vec3(x, y, z));
				}
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
		} else {
			mousePrev.x = xmouse;
			mousePrev.y = ymouse;
		}
	} else {
		mousePrev[0] = -1;
		mousePrev[1] = -1;
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

	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);

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

	
}

void render()
{
	// Update time.
	double t = glfwGetTime();
	
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
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_STRIP);
		for(int i = 0; i < ncps; ++i) {
			glVertex3f(cps[i].x, cps[i].y, cps[i].z);
		}
		glEnd();
	}

	// INSERT CODE HERE
	mat4* B;
	if (type == BEZIER) {
		B = &B_BEZIER;
	}
	else if (type == CATMULL_ROM) {
		B = &B_CATMULL_ROM;
	}
	else if (type == BASIS) {
		B = &B_BASIS;
	}

	// Fill in G column by column
	mat4 G;
	glColor3f(0.0f, 0.0f, 0.0f);
	// glLineWidth(3.0f);

	

	if (cps.size() >= 4) {
		// drawing curves
		glLineWidth(1.0f);
		for (int i = 0; i < cps.size()-3; ++i){
			glBegin(GL_LINE_STRIP);
			G[0] = glm::vec4(cps[i], 0.0f);
			G[1] = glm::vec4(cps[i+1], 0.0f);
			G[2] = glm::vec4(cps[i+2], 0.0f);
			G[3] = glm::vec4(cps[i+3], 0.0f);

			for(float u = 0.0f; u < 1.0f; u+=0.01f) {
				// Fill in uVec
				glm::vec4 uVec(1.0f, u, u*u, u*u*u);
				// Compute position at u
				glm::vec4 p = G*(*B*uVec);

				glVertex3f(p.x, p.y, p.z);
			}
			glEnd();
		}


		// frenet frame
		float kfloat;
		float speed = 0.5f;

		float u = std::modf(std::fmod(t*speed, ncps-3.0f), &kfloat);
		// printf("U: %f\n", u);
		int k = (int)std::floor(kfloat);
		G[0] = glm::vec4(cps[k], 0.0f);
		G[1] = glm::vec4(cps[k+1], 0.0f);
		G[2] = glm::vec4(cps[k+2], 0.0f);
		G[3] = glm::vec4(cps[k+3], 0.0f);

		vec4 uVec0 = vec4(1.0f, u, u*u, u*u*u);
		vec4 uVec1 = vec4(0.0f, 1.0f, 2.0f * u, 3.0f * u * u); // derivative
		vec4 uVec2 = vec4(0.0f, 0.0f, 2.0f , 6.0f * u ); // second derivative

		vec4 p_0 = G* (*B * uVec0);
		vec4 p_1 = G *(*B*uVec1); 
		vec4 p_2 = G *(*B*uVec2);

		vec4 tangent = p_1 / glm::length(p_1);
		vec4 cross_p1_p2 = vec4(glm::cross(vec3(p_1), vec3(p_2)), 0.0f);
		vec4 binorm = cross_p1_p2 / glm::length(cross_p1_p2);
		vec4 normal = vec4(glm::cross(vec3(tangent), vec3(binorm)), 0.0f);
		// p_1 = p_1 / glm::length(p_1); // normalized
		// p_2 = p_2 / glm::length(p_2); // normalized
		// cout << "p_1: (" << p_1.x << ", " << p_1.y << ", " << p_1.z << ")" << endl;
		if (keyToggles[(unsigned) 'd']) {
			cout << "p_0: " << endl;
			printVec4(p_0);
			for (int i = 0; i < cps.size(); ++i) {
				printVec4(vec4(cps[i], 0.0f));
			}
		}
		
		// tangent
		float magnitudeOfLines = 0.25f;
		glLineWidth(5.0f);
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_STRIP);
		for (float i = 0; i < magnitudeOfLines; i+=0.01f) {
			vec4 tan_p = p_0 + i * tangent;
			// vec4 binorm_p = p_0 + i * binorm;
			// vec4 binorm = p_0 + i 
			// glVertex3f(tmp.x, tmp.y, tmp.z);
			glVertex3f(tan_p.x, tan_p.y, tan_p.z);

		}
		glEnd();

		// binorm
		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_LINE_STRIP);
		for (float i = 0; i < magnitudeOfLines; i+=0.01f) {
			// vec4 tan_p = p_0 + i * tangent;
			vec4 binorm_p = p_0 + i * binorm;
			// vec4 binorm = p_0 + i 
			// glVertex3f(tmp.x, tmp.y, tmp.z);
			glVertex3f(binorm_p.x, binorm_p.y, binorm_p.z);

		}
		glEnd();

		// normal
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINE_STRIP);
		for (float i = 0; i < magnitudeOfLines; i+=0.01f) {
			// vec4 tan_p = p_0 + i * tangent;
			vec4 norm_p = p_0 + i * normal;
			// vec4 binorm = p_0 + i 
			// glVertex3f(tmp.x, tmp.y, tmp.z);
			glVertex3f(norm_p.x, norm_p.y, norm_p.z);

		}
		glEnd();





		// printf("G\n");
		// printMat4(G);

		// cout << "k: " << k << endl;
		// for (float _u = 0.0f; _u < 1.0f; _u+=0.01f) {

		// }
	}








	// DEBUG
	if (keyToggles[(unsigned)'d']) {

		printf("G\n");
		printMat4(G);
		printf("B %i\n", (int) type);
		printMat4(*B);
		keyToggles[(unsigned)'d'] = false;
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
