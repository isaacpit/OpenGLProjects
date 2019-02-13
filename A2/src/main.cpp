#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <map>

// CHANGED THIS from <GL/glew.h>
// CHANGED THIS FROM <GLFW/glfw3.h>
#define GLEW_STATIC
#include "../../glew-2.1.0/include/GL/glew.h"
#include "../../glfw-3.2.1/include/GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "GLSL.h"
#include "MatrixStack.h"
#include "Body.h"
#include "Part.h"




using namespace std;
using namespace glm;

// int KEY_X = 88;
// int KEY_Y = 89;
// int KEY_Z = 90;



GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from

GLuint progID;
map<string,GLint> attrIDs;
map<string,GLint> unifIDs;
map<string,GLuint> bufIDs;
int indCount;

// This function is called when a GLFW error occurs
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	// printf("key: %d scancode: %d action: %d mods: %d\n", key, scancode, action, mods);

	// Credit: https://gamedev.stackexchange.com/questions/71721/how-can-i-forward-glfws-keyboard-input-to-another-object
	Body * body = reinterpret_cast<Body *>(glfwGetWindowUserPointer(window));
	
	body->handleKey(key, mods);
	// cout << *body << endl;
	
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

}

// This function is called when the mouse is clicked
static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
	// Do nothing
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
	
	// Initialize time.
	glfwSetTime(0.0);
	
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
	string vShaderName = RESOURCE_DIR + "vert.glsl";
	string fShaderName = RESOURCE_DIR + "frag.glsl";
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
	attrIDs["aPos"] = glGetAttribLocation(progID, "aPos");
	attrIDs["aNor"] = glGetAttribLocation(progID, "aNor");
	
	// Get uniform IDs
	unifIDs["P"] = glGetUniformLocation(progID, "P");
	unifIDs["MV"] = glGetUniformLocation(progID, "MV");
	
	//
	// Vertex buffer setup
	//
	
	// Load OBJ geometry
	vector<float> posBuf;
	vector<float> norBuf;
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	string meshName = RESOURCE_DIR + "cube.obj";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;
	rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
		// Some OBJ files have different indices for vertex positions, normals,
		// and texture coordinates. For example, a cube corner vertex may have
		// three different normals. Here, we are going to duplicate all such
		// vertices.
		// Loop over shapes
		for(size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			size_t index_offset = 0;
			for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+0]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+1]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+2]);
					if(!attrib.normals.empty()) {
						norBuf.push_back(attrib.normals[3*idx.normal_index+0]);
						norBuf.push_back(attrib.normals[3*idx.normal_index+1]);
						norBuf.push_back(attrib.normals[3*idx.normal_index+2]);
					}
				}
				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}
	indCount = posBuf.size()/3; // number of indices to be rendered
	
	// Generate 2 buffer IDs and put them in the bufIDs map.
	GLuint tmp[2];
	glGenBuffers(2, tmp);
	bufIDs["bPos"] = tmp[0];
	bufIDs["bNor"] = tmp[1];
	
	glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bPos"]);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bNor"]);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	assert(norBuf.size() == posBuf.size());
	
	GLSL::checkError(GET_FILE_LINE);
}

// This function is called every frame to draw the scene.
static void render(Body* body)
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width/(float)height;

	// Set up projection matrix (camera intrinsics)
	mat4 P = perspective((float)(45.0*M_PI/180.0), aspect, 0.01f, 100.0f);
	
	// Modelview matrix: camera and world

	// MatrixStack MV;
	
	// Tell OpenGL which GLSL program to use
	glUseProgram(progID);
	// Pass in the current projection matrix
	glUniformMatrix4fv(unifIDs["P"], 1, GL_FALSE, value_ptr(P));
	// Enable the attribute
	glEnableVertexAttribArray(attrIDs["aPos"]);
	// Enable the attribute
	glEnableVertexAttribArray(attrIDs["aNor"]);
	// Bind the position buffer object to make it the currently active buffer
	glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bPos"]);
	// Set the pointer -- the data is already on the GPU
	glVertexAttribPointer(attrIDs["aPos"], 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	// Bind the color buffer object to make it the currently active buffer
	glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bNor"]);
	// Set the pointer -- the data is already on the GPU
	glVertexAttribPointer(attrIDs["aNor"], 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	
	// glColor3f(1.0f, 1.0f, 1.0f);
	// body.draw(unifIDs["MV"], 1, GL_FALSE, GL_TRIANGLES, 0, indCount);
	body->draw(unifIDs, indCount);

	// Unbind the buffer object
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Disable the attribute
	glDisableVertexAttribArray(attrIDs["aNor"]);
	// Disable the attribute
	glDisableVertexAttribArray(attrIDs["aPos"]);
	// Unbind our GLSL program
	glUseProgram(0);
	
	GLSL::checkError(GET_FILE_LINE);
}

void createFigure(Body* body) {

	vec3 upperRightArmJointScale(.3, .25, .3); 
	vec3 upperRightArmMeshScale(2, 1.5, .75);
	vec3 upperRightArmJointTrans(1.75, 1, 0);
	vec3 upperRightArmMeshTrans(.5, -.5, 0);

	// upper left arm
	vec3 upperLeftArmJointScale(upperRightArmJointScale[0], upperRightArmJointScale[1], upperRightArmJointScale[2]);
	vec3 upperLeftArmMeshScale(upperRightArmMeshScale[0], upperRightArmMeshScale[1], upperRightArmMeshScale[2]);
	vec3 upperLeftArmJointTrans(-upperRightArmJointTrans[0], upperRightArmJointTrans[1], upperRightArmJointTrans[2]);
	vec3 upperLeftArmMeshTrans(-upperRightArmMeshTrans[0], upperRightArmMeshTrans[1], upperRightArmMeshTrans[2]);

	// lower right arm
	vec3 lowerRightArmJointScale(.75, .75, 1); 
	vec3 lowerRightArmMeshScale(2, 1, .5);
	vec3 lowerRightArmJointTrans(3, -1, 0);
	vec3 lowerRightArmMeshTrans(.25, 0, 0);

	// lower left arm 
	vec3 lowerLeftArmJointScale(lowerRightArmJointScale[0], lowerRightArmJointScale[1], lowerRightArmJointScale[2]);
	vec3 lowerLeftArmMeshScale(lowerRightArmMeshScale[0], lowerRightArmMeshScale[1], lowerRightArmMeshScale[2]);
	vec3 lowerLeftArmJointTrans(-lowerRightArmJointTrans[0], lowerRightArmJointTrans[1], lowerRightArmJointTrans[2]);
	vec3 lowerLeftArmMeshTrans(-lowerRightArmMeshTrans[0], lowerRightArmMeshTrans[1], lowerRightArmMeshTrans[2]);

	// upper right leg
	vec3 upperRightLegJointScale(.4, .4, .5); 
	vec3 upperRightLegMeshScale(1, 2.5, 1);
	vec3 upperRightLegJointTrans(.15, -1, 0);
	vec3 upperRightLegMeshTrans(.5, -.5, 0);

	// upper left leg
	vec3 upperLeftLegJointScale(upperRightLegJointScale[0], upperRightLegJointScale[1], upperRightLegJointScale[2]);
	vec3 upperLeftLegMeshScale(upperRightLegMeshScale[0], upperRightLegMeshScale[1], upperRightLegMeshScale[2]);
	vec3 upperLeftLegJointTrans(-upperRightLegJointTrans[0], upperRightLegJointTrans[1], upperRightLegJointTrans[2]);
	vec3 upperLeftLegMeshTrans(-upperRightLegMeshTrans[0], upperRightLegMeshTrans[1], upperRightLegMeshTrans[2]);

	// lower right leg
	vec3 lowerRightLegJointScale(1, 1, .5); 
	vec3 lowerRightLegMeshScale(.75, 2, 1);
	vec3 lowerRightLegJointTrans(.55, -2.5, 0);
	vec3 lowerRightLegMeshTrans(0, -.4, 0);

	// lower left leg
	vec3 lowerLeftLegJointScale(lowerRightLegJointScale[0], lowerRightLegJointScale[1], lowerRightLegJointScale[2]);
	vec3 lowerLeftLegMeshScale(lowerRightLegMeshScale[0], lowerRightLegMeshScale[1], lowerRightLegMeshScale[2]);
	vec3 lowerLeftLegJointTrans(-lowerRightLegJointTrans[0], lowerRightLegJointTrans[1], lowerRightLegJointTrans[2]);
	vec3 lowerLeftLegMeshTrans(-lowerRightLegMeshTrans[0], lowerRightLegMeshTrans[1], lowerRightLegMeshTrans[2]);

	Part* worldView = new Part("worldView", vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 1, -10), vec3(0, 0, 0), nullptr, {}, false, -1);

	Part* torso = new Part("torso", vec3(2, 2, .1), vec3(1,1,1), vec3(0, 0, 0), vec3(0,0,0), worldView, {});

	Part* head = new Part("head", vec3(.5, .5, .5), vec3(1,1,1), vec3(0, 1.5, 0), vec3(0,0,0), torso, {});

	Part* upperRightArm = new Part("upperRightArm", upperRightArmJointScale, upperRightArmMeshScale, upperRightArmJointTrans, upperRightArmMeshTrans, torso, {});
	Part* lowerRightArm = new Part("lowerRightArm", lowerRightArmJointScale, lowerRightArmMeshScale, lowerRightArmJointTrans, lowerRightArmMeshTrans, upperRightArm, {});

	Part* upperLeftArm = new Part("upperLeftArm", upperLeftArmJointScale, upperLeftArmMeshScale, upperLeftArmJointTrans, upperLeftArmMeshTrans, torso, {});
	Part* lowerLeftArm = new Part("lowerLeftArm", lowerLeftArmJointScale, lowerLeftArmMeshScale, lowerLeftArmJointTrans, lowerLeftArmMeshTrans, upperLeftArm, {});

	Part* upperRightLeg = new Part("upperRightLeg", upperRightLegJointScale, upperRightLegMeshScale, upperRightLegJointTrans, upperRightLegMeshTrans, torso, {});
	Part* lowerRightLeg = new Part("lowerRightLeg", lowerRightLegJointScale, lowerRightLegMeshScale, lowerRightLegJointTrans, lowerRightLegMeshTrans, upperRightArm, {});

	Part* upperLeftLeg = new Part("upperLeftLeg", upperLeftLegJointScale, upperLeftLegMeshScale, upperLeftLegJointTrans, upperLeftLegMeshTrans, torso, {});
	Part* lowerLeftLeg = new Part("lowerLeftLeg", lowerLeftLegJointScale, lowerLeftLegMeshScale, lowerLeftLegJointTrans, lowerLeftLegMeshTrans, upperLeftLeg, {});

	body->setRoot(worldView);

	worldView->setChildren({torso});

	torso->setChildren({head, upperRightArm, upperLeftArm, upperRightLeg, upperLeftLeg});

	upperRightArm->setChildren({lowerRightArm});
	upperLeftArm->setChildren({lowerLeftArm});

	upperRightLeg->setChildren({lowerRightLeg});
	upperLeftLeg->setChildren({lowerLeftLeg});

	vector<Part*> parts = { torso, head, 
	upperRightArm, lowerRightArm, 
	upperLeftArm, lowerLeftArm, 
	upperRightLeg, lowerRightLeg,
	upperLeftLeg, lowerLeftLeg };

	body->setVectorParts(parts);
	body->setCurrPartIndex(0);

	cout << "tst" << endl;
	for (int i = 0; i < torso->children.size(); ++i) {
		cout << torso->children.at(i)->m_name << " ";
	}
	cout << endl;

	body->populateDeque();

	
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
	window = glfwCreateWindow(640, 480, "Isaac Pitblado", NULL, NULL);
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

	// Create handler object
	Body* body = new Body();
	createFigure(body);

	glfwSetWindowUserPointer(window, body);


	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render(body);
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
