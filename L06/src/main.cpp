#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>

#define GLEW_STATIC
#include "../../glew-2.1.0/include/GL/glew.h"
#include "../../glfw-3.2.1/include/GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSL.h"

using namespace std;
using namespace glm;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from

GLuint progID;
GLint PUnifID;
GLint HeightUnifID;
GLint WidthUnifID;
GLint MovingCenterUnifID;
GLint vertPosAttrID;
GLuint posBufID;

float worldSize = 1.2f;
int NUM_TRIANGLES = 9;

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
static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
	double posX, posY;
	float newPt[2];
	if(action == GLFW_PRESS) {
		// Get window size
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		// Get cursor position
		glfwGetCursorPos(window, &posX, &posY);
		posY = height - posY; // flip the Y-axis
		cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		// Compute where the cursor is in world coords
		float aspect = width/(float)height;
		float xMinWorld, xMaxWorld, yMinWorld, yMaxWorld;
		if(width > height) {
			xMinWorld = -worldSize*aspect;
			xMaxWorld =  worldSize*aspect;
			yMinWorld = -worldSize;
			yMaxWorld =  worldSize;
		} else {
			xMinWorld = -worldSize;
			xMaxWorld =  worldSize;
			yMinWorld = -worldSize/aspect;
			yMaxWorld =  worldSize/aspect;
		}
		// At this point:
		// - The lower left corner of the world is (xMinWorld, xMaxWorld)
		// - The top right corner of the world is (yMinWorld, yMaxWorld)
		// - The lower left corner of the window is (0, 0)
		// - The top right corner of the window is (width, height)
		//
		// THIS IS BROKEN - YOU GET TO FIX IT - yay!
		float s_x = (xMaxWorld - xMinWorld) / width;
		float s_y = (yMaxWorld - yMinWorld) / height;
		float t_x = xMinWorld;
		float t_y = yMinWorld;

		cout << "s (" << s_x << ", " << s_y << endl;
		cout << "t (" << t_x << ", " << t_y << endl;
		newPt[0] = s_x * posX + t_x;
		newPt[1] = s_y * posY + t_y;
		cout << "converted: " << newPt[0] << " " << newPt[1] << endl;
		// Update the vertex array with newPt
		glBindBuffer(GL_ARRAY_BUFFER, posBufID);
		glBufferSubData(GL_ARRAY_BUFFER, 6*sizeof(float), 2*sizeof(float), newPt);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// This function is called once to initialize the scene and OpenGL
static void init()
{
	//
	// General setup
	//
	
	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	//
	// GLSL program setup
	//
	
	// Create shader handles
	GLuint vShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	
	// Read shader sources
	string vShaderName = RESOURCE_DIR + "simple_vert.glsl";
	string fShaderName = RESOURCE_DIR + "simple_frag.glsl";
	const char *vShaderText = GLSL::textFileRead(vShaderName.c_str());
	const char *fShaderText = GLSL::textFileRead(fShaderName.c_str());
	glShaderSource(vShaderID, 1, &vShaderText, NULL);
	glShaderSource(fShaderID, 1, &fShaderText, NULL);
	
	// Compile vertex shader
	int rc;
	glCompileShader(vShaderID);
	glGetShaderiv(vShaderID, GL_COMPILE_STATUS, &rc);
	if(!rc) {
		GLSL::printShaderInfoLog(vShaderID);
		cout << "Error compiling vertex shader " << vShaderName << endl;
		return;
	}
	
	// Compile fragment shader
	glCompileShader(fShaderID);
	glGetShaderiv(fShaderID, GL_COMPILE_STATUS, &rc);
	if(!rc) {
		GLSL::printShaderInfoLog(fShaderID);
		cout << "Error compiling fragment shader " << fShaderName << endl;
		return;
	}
	
	// Create the program and link
	progID = glCreateProgram();
	glAttachShader(progID, vShaderID);
	glAttachShader(progID, fShaderID);
	glLinkProgram(progID);
	glGetProgramiv(progID, GL_LINK_STATUS, &rc);
	if(!rc) {
		GLSL::printProgramInfoLog(progID);
		cout << "Error linking shaders " << vShaderName << " and " << fShaderName << endl;
		return;
	}
	
	// Get vertex attribute IDs
	vertPosAttrID = glGetAttribLocation(progID, "vertPos");
	
	// Get uniform IDs
	PUnifID = glGetUniformLocation(progID, "P");
	HeightUnifID = glGetUniformLocation(progID, "height");
	WidthUnifID = glGetUniformLocation(progID, "width");
	MovingCenterUnifID = glGetUniformLocation(progID, "movingCenter");
	
	//
	// Vertex buffer setup
	//
	
	// Vertex position data
	vector<float> posBuf;
	posBuf.push_back(-1.0f); // x
	posBuf.push_back(-1.0f); // y
	posBuf.push_back( 0.0f); // z
	posBuf.push_back( 1.0f); // x
	posBuf.push_back(-1.0f); // y
	posBuf.push_back( 0.0f); // z
	posBuf.push_back( 0.0f); // x
	posBuf.push_back( 1.0f); // y
	posBuf.push_back( 0.0f); // z


	posBuf.push_back(-1.05f); // x
	posBuf.push_back(-1.0f); // y
	posBuf.push_back( 0.0f); // z
	posBuf.push_back(-0.05f); // x
	posBuf.push_back( 1.0f); // y
	posBuf.push_back( 0.0f); // z
	posBuf.push_back(-1.05f); // x
	posBuf.push_back( 1.0f); // y
	posBuf.push_back( 0.0f); // z

	posBuf.push_back(1.05f); // x
	posBuf.push_back(-1.0f); // y
	posBuf.push_back( 0.0f); // z
	posBuf.push_back( 0.05f); // x
	posBuf.push_back( 1.0f); // y
	posBuf.push_back( 0.0f); // z
	posBuf.push_back( 1.05f); // x
	posBuf.push_back( 1.0f); // y
	posBuf.push_back( 0.0f); // z 
	
	// Generate a buffer object
	glGenBuffers(1, &posBufID);
	// Bind the buffer object to make it the currently active buffer
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	// Send the buffer data
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_DYNAMIC_DRAW);
	// Unbind the buffer object
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// This function is called every frame to draw the scene.
static void render()
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width/(float)height;



	// Set up projection matrix (camera intrinsics)
	mat4 P;
	if(width > height) {
		P = ortho(-worldSize*aspect, worldSize*aspect, -worldSize, worldSize, -1.0f, 1.0f);
	} else {
		P = ortho(-worldSize, worldSize, -worldSize/aspect, worldSize/aspect, -1.0f, 1.0f);
	}

	float t = glfwGetTime();
	vec2 center = vec2(width /2, height/2);
	float radius = 300.0;
	vec2 movingCenter = vec2(center[0] + radius * cos(t), center[1] + radius * sin(t));
	// vec2 movingCenter = center * t;
	// cout << "t: " << t << endl;
	// cout << "center: (" << (width /2) << ", " << (height /2) << ")" << endl;
	// cout << "movingCenter: (" <<  movingCenter[1] << ", " << movingCenter[1] << ")" << endl;
	// Tell OpenGL which GLSL program to use
	glUseProgram(progID);
	// Pass in the current projection matrix
	glUniformMatrix4fv(PUnifID, 1, GL_FALSE, &P[0][0]);

	// Pass in the height of the window
	glUniform1i(HeightUnifID, height);
	glUniform1i(WidthUnifID, width);
	glUniform2fv(MovingCenterUnifID, 1, &movingCenter[0]);

	// remove
	// GLSL::checkError(GET_FILE_LINE);

	// Enable the attribute
	glEnableVertexAttribArray(vertPosAttrID);
	// Bind the position buffer object to make it the currently active buffer
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	// Set the pointer -- the data is already on the GPU
	glVertexAttribPointer(vertPosAttrID, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	
	// Actually draw here
	glDrawArrays(GL_TRIANGLES, 0, NUM_TRIANGLES);
	// Unbind the buffer object
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Disable the attribute
	glDisableVertexAttribArray(vertPosAttrID);
	// Unbind our GLSL program
	glUseProgram(0);
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
	// Set the mouse call back.
	glfwSetMouseButtonCallback(window, mouse_callback);
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
