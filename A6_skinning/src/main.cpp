#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <memory>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
// #include <glm/gtdc/type_ptr.hpp>

#include "GLSL.h"
#include "Program.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "ShapeSkin.h"

using namespace std;

GLFWwindow *window;				// Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
string MESH_FILE = "";
string ATTACHMENT_FILE = "";
string SKELETON_FILE = "";
bool keyToggles[256] = {false};

shared_ptr<Camera> camera = NULL;
vector<shared_ptr<ShapeSkin > > allShapes;
shared_ptr<ShapeSkin> shape0 = NULL;
shared_ptr<ShapeSkin> shape1 = NULL;
shared_ptr<ShapeSkin> shape2 = NULL;
shared_ptr<ShapeSkin> shape3 = NULL;
shared_ptr<ShapeSkin> shape4 = NULL;
shared_ptr<ShapeSkin> shape5 = NULL;
shared_ptr<Program> progSimple = NULL;
shared_ptr<Program> progSkin = NULL;
shared_ptr<Program> progCalc = NULL;

vector<vector<int>> shapeMode;
int maxGPU = 5;
int maxCPU = 2;

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];
	switch (key)
	{
	case 'g':
		break;
	}
}

static void cursor_position_callback(GLFWwindow *window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS)
	{
		camera->mouseMoved(xmouse, ymouse);
	}
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (action == GLFW_PRESS)
	{
		bool shift = mods & GLFW_MOD_SHIFT;
		bool ctrl = mods & GLFW_MOD_CONTROL;
		bool alt = mods & GLFW_MOD_ALT;
		camera->mouseClicked(xmouse, ymouse, shift, ctrl, alt);
	}
}

void loadScene(const string &meshFile, const string &attachmentFile)
{
	keyToggles[(unsigned)'c'] = true;

	camera = make_shared<Camera>();

	// int nShapes = 3;
	// allShapes = vector< shared_ptr<ShapeSkin> > (nShapes);
	// Single shape for all the animations.
	shape0 = make_shared<ShapeSkin>();
	shape0->loadMesh(meshFile);
	shape0->loadAttachment(attachmentFile);
	shape0->loadSkeleton(SKELETON_FILE);
	allShapes.push_back(shape0);

	shape1 = make_shared<ShapeSkin>();
	shape1->loadMesh(meshFile);
	shape1->loadAttachment(attachmentFile);
	shape1->loadSkeleton("../cheb/cheb_skel_jumpAround.txt");
	allShapes.push_back(shape1);

	shape2 = make_shared<ShapeSkin>();
	shape2->loadMesh(meshFile);
	shape2->loadAttachment(attachmentFile);
	shape2->loadSkeleton("../cheb/cheb_skel_crossWalk.txt");
	allShapes.push_back(shape2);

	shape3 = make_shared<ShapeSkin>();
	shape3->loadMesh(meshFile);
	shape3->loadAttachment(attachmentFile);
	shape3->loadSkeleton("../cheb/cheb_skel_walk.txt");
	allShapes.push_back(shape3);

	shape4 = make_shared<ShapeSkin>();
	shape4->loadMesh(meshFile);
	shape4->loadAttachment(attachmentFile);
	shape4->loadSkeleton("../cheb/cheb_skel_walkAndSkip.txt");
	allShapes.push_back(shape4);

	shape5 = make_shared<ShapeSkin>();
	shape5->loadMesh(meshFile);
	shape5->loadAttachment(attachmentFile);
	shape5->loadSkeleton("../cheb/cheb_skel_runAround.txt");
	allShapes.push_back(shape5);


	

	int nShapes = allShapes.size();


	// For drawing the grid, etc.
	progSimple = make_shared<Program>();
	progSimple->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	progSimple->setVerbose(true);

	// For skinned shape0, CPU/GPU
	progSkin = make_shared<Program>();
	progSkin->setShaderNames(RESOURCE_DIR + "skin_vert.glsl", RESOURCE_DIR + "skin_frag.glsl");
	progSkin->setVerbose(true);

	progCalc = make_shared<Program>();
	progCalc->setShaderNames(RESOURCE_DIR + "calc_skin_vert.glsl", RESOURCE_DIR + "calc_skin_frag.glsl");
	progCalc->setVerbose(true);


	int mx = (maxGPU > maxCPU) ? maxGPU : maxCPU;
	shapeMode = vector<vector<int>>(mx);
	cerr << "# shapes: " << nShapes << endl;
	for (int i = 0; i < mx; ++i)
	{
		shapeMode.at(i) = vector<int>(mx);
		for (int j = 0; j < mx; ++j)
		{
			shapeMode.at(i).at(j) = rand() % nShapes;
			cerr << shapeMode.at(i).at(j) << " ";
		}
		cerr << endl;
	}
}

void init()
{
	// Non-OpenGL things
	loadScene(MESH_FILE, ATTACHMENT_FILE);

	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shape0->init();
	shape1->init();
	shape2->init();
	shape3->init();
	shape4->init();
	shape5->init();

	progSimple->init();
	progSimple->addUniform("P");
	progSimple->addUniform("MV");

	progSkin->init();
	progSkin->addAttribute("aPos");
	progSkin->addAttribute("aNor");
	progSkin->addUniform("P");
	progSkin->addUniform("MV");

	progCalc->init();
	progCalc->addAttribute("aPos");
	progCalc->addAttribute("aNor");
	progCalc->addAttribute("weights0");
	progCalc->addAttribute("weights1");
	progCalc->addAttribute("weights2");
	progCalc->addAttribute("weights3");
	progCalc->addAttribute("bones0");
	progCalc->addAttribute("bones1");
	progCalc->addAttribute("bones2");
	progCalc->addAttribute("bones3");
	progCalc->addAttribute("numInfl");
	progCalc->addUniform("P");
	progCalc->addUniform("MV");
	progCalc->addUniform("animMat");
	progCalc->addUniform("bindMatInv");

	// Initialize time.
	glfwSetTime(0.0);

	GLSL::checkError(GET_FILE_LINE);
}

void drawGrid(shared_ptr<MatrixStack> MV, shared_ptr<MatrixStack> P, float t)
{
	// Draw grid
	progSimple->bind();
	glUniformMatrix4fv(progSimple->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(progSimple->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	float gridSizeHalf = 5.0f;
	int gridNx = 11;
	int gridNz = 11;
	glLineWidth(1);
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_LINES);
	for (int i = 0; i < gridNx; ++i)
	{
		float alpha = i / (gridNx - 1.0f);
		float x = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;
		glVertex3f(x, 0, -gridSizeHalf);
		glVertex3f(x, 0, gridSizeHalf);
	}
	for (int i = 0; i < gridNz; ++i)
	{
		float alpha = i / (gridNz - 1.0f);
		float z = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;
		glVertex3f(-gridSizeHalf, 0, z);
		glVertex3f(gridSizeHalf, 0, z);
	}
	glEnd();

	progSimple->unbind();
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
	camera->setAspect((float)width / (float)height);

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (keyToggles[(unsigned)'c'])
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
	if (keyToggles[(unsigned)'z'])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	bool CPU = !keyToggles[(unsigned)'g'];

	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();

	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	camera->applyViewMatrix(MV);

	drawGrid(MV, P, t);

	// MV->pushMatrix();
	// progSimple->bind();

	// progSimple->unbind();
	// MV->popMatrix();
	shared_ptr<Program> prog = (CPU) ? progSkin : progCalc;

	// Draw character
	MV->pushMatrix();
	prog->bind();
	MV->pushMatrix();
	// MV->scale();

	int mx = (CPU) ? maxCPU : maxGPU;

	for (int i = 0; i < mx; ++i)
	{
		for (int j = 0; j < mx; ++j)
		{
			MV->pushMatrix();
			MV->translate(-2 * (float)mx/(float)2, 0, -2 * (float)mx/(float)2);
			MV->translate(2 * i, 0, 2 * j);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			// if (!CPU) {
			// 	// m = shape->sendAnimationMatrices(t)
			// }

			MV->popMatrix();
			auto currShape = allShapes.at(shapeMode.at(i).at(j));
			currShape->setProgram(prog);
			currShape->drawBindPoseFrenetFrames(MV, t, keyToggles[(unsigned)'d'], .05);
			currShape->drawAnimationFrenetFrames(MV, t, keyToggles[(unsigned)'d'], .05, CPU);
			currShape->draw(CPU);
			// if (CPU) shape->setProgram(progSkin);
			// else shape->setProgram(progCalc);
			// printf("(i, j): (%d, %d) \n", i, j);
			// if (shapeMode.at(i).at(j) == 0)
			// {
			// 	shape0->setProgram(prog);
			// 	shape0->drawBindPoseFrenetFrames(MV, t, keyToggles[(unsigned)'d'], .05);
			// 	shape0->drawAnimationFrenetFrames(MV, t, keyToggles[(unsigned)'d'], .05, CPU);
			// 	shape0->draw(CPU);
			// }
			// else if (shapeMode.at(i).at(j) == 1)
			// {
			// 	shape1->setProgram(prog);
			// 	shape1->drawBindPoseFrenetFrames(MV, t, keyToggles[(unsigned)'d'], .05);
			// 	shape1->drawAnimationFrenetFrames(MV, t, keyToggles[(unsigned)'d'], .05, CPU);
			// 	shape1->draw(CPU);
			// }
		}
	}
	// glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	// glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	// // if (!CPU) {
	// // 	// m = shape->sendAnimationMatrices(t)
	// // }

	// MV->popMatrix();

	// // if (CPU) shape->setProgram(progSkin);
	// // else shape->setProgram(progCalc);
	// shape->setProgram(prog);
	// if (!CPU) {

	// }
	// shape->drawBindPoseFrenetFrames(MV, t, keyToggles[(unsigned)'d'], .05);
	// shape->drawAnimationFrenetFrames(MV, t, keyToggles[(unsigned)'d'], .05, CPU);

	// shape->draw(CPU);
	prog->unbind();
	MV->popMatrix();

	// Pop matrix stacks.
	MV->popMatrix();
	P->popMatrix();

	GLSL::checkError(GET_FILE_LINE);

	keyToggles[(unsigned)'d'] = false;
}

int main(int argc, char **argv)
{
	if (argc < 5)
	{
		cout << "Usage: Assignment2 <SHADER DIR> <MESH FILE> <ATTACHMENT FILE> <SKELETON FILE>" << endl;
		return 0;
	}

	srand(NULL);

	RESOURCE_DIR = argv[1] + string("/");
	MESH_FILE = argv[2];
	ATTACHMENT_FILE = argv[3];
	SKELETON_FILE = argv[4];

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if (!glfwInit())
	{
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "YOUR NAME", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
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
	while (!glfwWindowShouldClose(window))
	{
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
