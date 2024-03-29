#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Texture.h"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from

shared_ptr<Camera> camera;
shared_ptr<Program> prog;
shared_ptr<Texture> texture0;
shared_ptr<Texture> texture1;
shared_ptr<Texture> texture2;
shared_ptr<Shape> shape;

glm::mat3 T;
glm::vec3 lightPosCam;

bool keyToggles[256] = {false}; // only for English keyboards!
float pos[2] = {0};

int KEY_W = 119;
int KEY_A = 97;
int KEY_S = 115;
int KEY_D = 100;

float STEP = .01;

float RATIO = .000001;

void handleTranslate(int key) {
	if (key == KEY_W) {
		pos[1] = pos[1] - STEP ;
	}
	else if (key == KEY_A) {
		pos[0] = pos[0] + STEP ;
	}
	else if (key == KEY_S) {
		pos[1] = pos[1] + STEP ;
	}
	else if (key == KEY_D) {
		pos[0] = pos[0] - STEP ;
	}
	// cout << "(x,y): (" << pos[0] << ", " << pos[1] << ")" << endl;
}

// This function is called when a GLFW error occurs
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

// This function is called when the mouse is clicked
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(action == GLFW_PRESS) {
		bool shift = (mods & GLFW_MOD_SHIFT) != 0;
		bool ctrl  = (mods & GLFW_MOD_CONTROL) != 0;
		bool alt   = (mods & GLFW_MOD_ALT) != 0;
		camera->mouseClicked((float)xmouse, (float)ymouse, shift, ctrl, alt);
	}
}

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		camera->mouseMoved((float)xmouse, (float)ymouse);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];
	switch(key) {
		case 'x':
			lightPosCam.x += 0.1;
			break;
		case 'X':
			lightPosCam.x -= 0.1;
			break;
		case 'y':
			lightPosCam.y += 0.1;
			break;
		case 'Y':
			lightPosCam.y -= 0.1;
			break;
	}
	handleTranslate(key);
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// This function is called once to initialize the scene and OpenGL
static void init()
{
	// Initialize time.
	glfwSetTime(0.0);

	// Spin globe initially
	keyToggles['t'] = true;
	
	// Set background color.
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	prog = make_shared<Program>();
	prog->setShaderNames(RESOURCE_DIR + "vert.glsl", RESOURCE_DIR + "frag.glsl");
	prog->setVerbose(true);
	prog->init();
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->addAttribute("aTex");
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addUniform("T");
	prog->addUniform("texture0");
	prog->addUniform("texture1");
	prog->addUniform("texture2");
	prog->addUniform("lightPosCam");
	prog->setVerbose(false);
	
	
	camera = make_shared<Camera>();
	camera->setInitDistance(3.0f);
	
	texture0 = make_shared<Texture>();
	texture0->setFilename(RESOURCE_DIR + "earthKd.jpg");
	texture0->init();
	texture0->setUnit(0);
	texture0->setWrapModes(GL_REPEAT, GL_REPEAT);
	
	texture1 = make_shared<Texture>();
	texture1->setFilename(RESOURCE_DIR + "earthKs.jpg");
	texture1->init();
	texture1->setUnit(1);
	texture1->setWrapModes(GL_REPEAT, GL_REPEAT);
	
	texture2 = make_shared<Texture>();
	texture2->setFilename(RESOURCE_DIR + "earthClouds.jpg");
	texture2->init();
	texture2->setUnit(2);
	texture2->setWrapModes(GL_REPEAT, GL_REPEAT);

	T[0][0] = 1.0f;
	T[1][1] = 1.0f;
	T[2][2] = 1.0f;

	// for (int i = 0; i < 3; ++i) {
	// 	for (int j = 0; j < 3; ++j) {
	// 		cout << "T[" << i << "][" << j << "]: " << T[i][j] << endl;
	// 	}
	// }
	
	lightPosCam.x = 1.0f;
	lightPosCam.y = 1.0f;
	lightPosCam.z = 1.0f;
	
	shape = make_shared<Shape>();
	shape->loadMesh(RESOURCE_DIR + "sphere.obj");
	shape->init();
	
	GLSL::checkError(GET_FILE_LINE);
}

// This function is called every frame to draw the scene.
static void render()
{
	// Clear framebuffer.
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
	if (keyToggles[(unsigned)'t']) {
		float t = glfwGetTime();
		pos[0] += RATIO * t;
		pos[1] += RATIO * t;
	}

	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	camera->applyViewMatrix(MV);

	T[0][2] = pos[0];
	T[1][2] = pos[1];
	
	prog->bind();
	texture0->bind(prog->getUniform("texture0"));
	texture1->bind(prog->getUniform("texture1"));
	texture2->bind(prog->getUniform("texture2"));
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glUniformMatrix3fv(prog->getUniform("T"), 1, GL_FALSE, glm::value_ptr(T));
	glUniform3fv(prog->getUniform("lightPosCam"), 1, glm::value_ptr(lightPosCam));
	shape->draw(prog);
	texture1->unbind();
	texture0->unbind();
	prog->unbind();
	
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
	GLSL::checkVersion();
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
	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);
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
