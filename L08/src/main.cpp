#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

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

#include "Material.h"
#include "Light.h"

using namespace std;

struct InputManager {
	Material* mats;
	Light* lights;
	bool isPhong = true;
};

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from

shared_ptr<Camera> camera;
shared_ptr<Program> prog;
shared_ptr<Program> prog_sil;
shared_ptr<Shape> shape;

int KEY_M_MAIN = 77;
int KEY_L_MAIN = 76;
int KEY_X_MAIN = 88;
int KEY_Y_MAIN = 89;
int KEY_S_MAIN = 83;
// int KEY_M_MAIN = 77;

bool keyToggles[256] = {false}; // only for English keyboards!

// This function is called when a GLFW error occurs
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	// printf("key: %d | scancode: %d |  action: %d | mods: %d\n", key, scancode, action, mods);

	InputManager * inputManager = reinterpret_cast<InputManager *>(glfwGetWindowUserPointer(window));


	if (key == KEY_M_MAIN) {
		inputManager->mats->handleKey(key, mods);
	}
	else if (key == KEY_L_MAIN || key == KEY_X_MAIN || key == KEY_Y_MAIN) {
		inputManager->lights->handleKey(key, mods);
	}
	else if (key == KEY_S_MAIN && action == 1) {
		inputManager->isPhong = !inputManager->isPhong;
	}


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
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// This function is called once to initialize the scene and OpenGL
static void init(string objFile)
{
	// Initialize time.
	glfwSetTime(0.0);
	
	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	prog_sil = make_shared<Program>();
	prog_sil->setShaderNames(RESOURCE_DIR + "sil_vert.glsl", RESOURCE_DIR + "sil_frag.glsl");
	prog_sil->setVerbose(true);
	prog_sil->init();
	prog_sil->addAttribute("aPos");
	prog_sil->addAttribute("aNor");
	prog_sil->addUniform("MV");
	prog_sil->addUniform("P");
	prog_sil->setVerbose(false);

	prog = make_shared<Program>();
	prog->setShaderNames(RESOURCE_DIR + "vert.glsl", RESOURCE_DIR + "frag.glsl");
	prog->setVerbose(true);
	prog->init();
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->addUniform("MV");
	prog->addUniform("P");
	prog->addUniform("lightPos1");
	prog->addUniform("lightPos2");
	prog->addUniform("intensity1");
	prog->addUniform("intensity2");
	
	prog->addUniform("ka");
	prog->addUniform("kd");
	prog->addUniform("ks");
	prog->addUniform("s");
	prog->setVerbose(false);
	
	camera = make_shared<Camera>();
	camera->setInitDistance(2.0f);
	
	shape = make_shared<Shape>();
	shape->loadMesh(RESOURCE_DIR + objFile);
	shape->fitToUnitBox();
	shape->init();
	
	GLSL::checkError(GET_FILE_LINE);
}

// This function is called every frame to draw the scene.
static void render(Material* mats, Light* lights, InputManager* inputManager)
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	// if(keyToggles[(unsigned)'l']) {
	// 	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// } else {
	// 	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// }
	
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

	if (inputManager->isPhong) {
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniform3f(prog->getUniform("lightPos1"), lights->lights[0]->pos.x, lights->lights[0]->pos.y, lights->lights[0]->pos.z);
		glUniform3f(prog->getUniform("lightPos2"), lights->lights[1]->pos.x, lights->lights[1]->pos.y, lights->lights[1]->pos.z);
		glUniform1f(prog->getUniform("intensity1"), lights->lights[0]->intensity);
		glUniform1f(prog->getUniform("intensity2"), lights->lights[1]->intensity);

		MatNode* currMat = mats->getCurrMat();
		vec3 currKa  = currMat->ka;
		vec3 currKd  = currMat->kd;
		vec3 currKs  = currMat->ks;
		float currS = currMat->s;

		glUniform3f(prog->getUniform("ka"), currKa.x, currKa.y, currKa.z);
		glUniform3f(prog->getUniform("kd"), currKd.x, currKd.y, currKd.z);
		glUniform3f(prog->getUniform("ks"), currKs.x, currKs.y, currKs.z);
		glUniform1f(prog->getUniform("s"), currS);

		shape->draw(prog);
		prog->unbind();
	}
	else {
		prog_sil->bind();
		glUniformMatrix4fv(prog_sil->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog_sil->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		shape->draw(prog_sil);
		prog_sil->unbind();
	}
	
	
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

Material* initMaterials() {
	Material* mats = new Material();

	vec3 nWhite_ka(0.2f, 0.2f, 0.2f);
	vec3 nWhite_kd(0.8f, 0.7f, 0.7f);
	vec3 nWhite_ks(1.0f, 0.9f, 0.8f);
	float nWhite_s = 200.0f;
	MatNode* nWhite = new MatNode(nWhite_ka, nWhite_kd, nWhite_ks, nWhite_s);

	vec3 nBlue_ka(0.0f, 0.2f, 0.0f);
	vec3 nBlue_kd(0.05f, 0.1f, 1.0f);
	vec3 nBlue_ks(1.0f, 0.9f, 0.8f);
	float nBlue_s = 200.0f;
	MatNode* nBlue = new MatNode(nBlue_ka, nBlue_kd, nBlue_ks, nBlue_s);

	vec3 nGray_ka(0.1f, 0.1f, 0.f);
	vec3 nGray_kd(0.3f, 0.3f, 0.5f);
	vec3 nGray_ks(0.1f, 0.1f, 0.25f);
	float nGray_s = 25.0f;
	MatNode* nGray = new MatNode(nGray_ka, nGray_kd, nGray_ks, nGray_s);

	mats->addMatNode(nWhite);
	mats->addMatNode(nBlue);
	mats->addMatNode(nGray);

	return mats;
}

Light* initLights() {
	Light* lights = new Light();

	vec3 light1_vec = vec3(1.0f, 1.0f, 1.0f);
	float light1_intensity = 0.8f;
	LightNode* light1 = new LightNode(light1_vec, light1_intensity);

	vec3 light2_vec = vec3(-1.0f, 1.0f, 1.0f);
	float light2_intensity = 0.2f;
	LightNode* light2 = new LightNode(light2_vec, light2_intensity);

	lights->addLightNode(light1);
	lights->addLightNode(light2);

	return lights;

}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");

	string objFile = "";
	if (argc < 3) {
		cout << "Selecting the bunny.obj by default" << endl;
		objFile = "bunny.obj";
	}
	else {
		objFile = argv[2];
	}

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
	init(objFile);
	// Initialize materials.
	Material* mats = initMaterials();

	Light* lights = initLights();
	lights->printLights();

	InputManager* inputManager = new InputManager();
	inputManager->lights = lights;
	inputManager->mats = mats;

	// Allows access of Materials pointer in key callback
	glfwSetWindowUserPointer(window, inputManager);

	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render(mats, lights, inputManager);
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
