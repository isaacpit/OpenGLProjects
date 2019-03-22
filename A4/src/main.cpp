#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <memory>
#include <cstdlib>

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

using glm::vec4;


struct Coords {
	int r = 5;
	int c = 5;
	int max = 4;
	int min = - 4;
	float** worldSpaceX;
	float** worldSpaceZ;
	float** whichShape;

	vector<vector<vec3>> colors;
	vector<vector<vec3>> ks;

	float spacing = 2;
	// float** worldSpaceZ;

	Coords() {
		r = (max-min) / spacing + 1;
		c = r;
		cout << "max: " << max << endl;
		cout << "min: " << min << endl;
		cout << "spacing: " << spacing << endl;
		cout << "rows: " << r << endl;
		cout << "cols: " << c << endl;
		
		worldSpaceX = new float*[r];
		worldSpaceZ = new float*[r];
		whichShape = new float*[r];

		colors = vector<vector<vec3>>(r);
		ks = vector<vector<vec3>>(r);

		for (int i = 0; i < r; ++i) {
			worldSpaceX[i] = new float[c];
			worldSpaceZ[i] = new float[c];
			whichShape[i] = new float[c];
			colors.at(i) = vector<vec3>(c);
			ks.at(i) = vector<vec3>(c);
		}
		int k = 0;
		int l = 0;
		for (int i = min; i <= max ; i += spacing) {
			cout << " i j | (xzS) [rgb] [[ks]] \n";
			for (int j = min; j <= max; j += spacing) {
				float r = ( rand() % (10000-0) )/ 10000.0;
				float g = ( rand() % (10000-0) )/ 10000.0;
				float b = ( rand() % (10000-0) )/ 10000.0;


				float ks_r = ( rand() % (10000-0) )/ 10000.0;
				float ks_g = ( rand() % (10000-0) )/ 10000.0;
				float ks_b = ( rand() % (10000-0) )/ 10000.0;




				worldSpaceX[k][l] = j ;
				worldSpaceZ[k][l] = - i ;
				whichShape[k][l] = rand() % 2;
				colors.at(k).at(l) = vec3(r, g, b);
				ks.at(k).at(l) = vec3(ks_r, ks_g, ks_b);
				
				printf("%d %d | (%.2f, %.2f, %.2f) [%.2f, %.2f, %.2f] [[%.2f, %.2f, %.2f]] \n", k, l, worldSpaceX[k][l], worldSpaceZ[k][l], whichShape[k][l], colors[k][l].r, colors[k][l].g, colors[k][l].b,  ks[k][l].r, ks[k][l].g, ks[k][l].b);
				++l; 
			} cout << endl;
			l = 0;
			++k;
		}
	}


};

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

struct InputManager {
	enum Mode {
		Phong, Silhouette, Cel, LAST
	};
	Material* mats;
	Light* lights;
	CelMat* celMats;
	Mode mode = Mode::Phong;
	Coords* coords;

	void nextMode() {
		if (mode == Mode::Phong) {
			mode = Silhouette;
		}
		else if (mode == Mode::Silhouette) {
			mode = Cel;
		}
		else if (mode == Mode::Cel) {
			mode = Phong;
		}
	}
	void prevMode() {
		if (mode == Mode::Phong) {
			mode = Cel;
		}
		else if (mode == Mode::Silhouette) {
			mode = Phong;
		}
		else if (mode == Mode::Cel) {
			mode = Silhouette;
		}
	}

};



GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from

shared_ptr<Camera> camera;
shared_ptr<Program> prog;
shared_ptr<Program> prog_sil;
shared_ptr<Program> prog_cel;
shared_ptr<Shape> shape;
shared_ptr<Shape> shape1;

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


	if ((key == KEY_M_MAIN) && inputManager->mode == InputManager::Mode::Phong) {
		inputManager->mats->handleKey(key, mods);
	}
	else if ((key == KEY_L_MAIN || key == KEY_X_MAIN || key == KEY_Y_MAIN) && inputManager->mode == InputManager::Mode::Phong) {
		inputManager->lights->handleKey(key, mods);
	}
	else if (key == KEY_S_MAIN && action == 1 && mods == 0) {
		inputManager->nextMode();
	}
	else if (key == KEY_S_MAIN && action == 1 && mods == 1) {
		inputManager->prevMode();
	}
	else if (key == KEY_M_MAIN && action == 1 && mods == 0 && inputManager->mode == InputManager::Mode::Cel) {
		inputManager->celMats->nextCelMat();
	}
		else if (key == KEY_M_MAIN && action == 1 && mods == 1 && inputManager->mode == InputManager::Mode::Cel) {
		inputManager->celMats->prevCelMat();
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
static void init(string objFile[])
{
	cout << "objFile0: " << objFile[0] << endl;
	cout << "objFile1: " << objFile[1] << endl;
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


	prog_cel = make_shared<Program>();
	prog_cel->setShaderNames(RESOURCE_DIR + "cel_vert.glsl", RESOURCE_DIR + "cel_frag.glsl");
	prog_cel->setVerbose(true);
	prog_cel->init();
	prog_cel->addAttribute("aPos");
	prog_cel->addAttribute("aNor");
	prog_cel->addUniform("MV");
	prog_cel->addUniform("P");
	prog_cel->addUniform("c1");
	prog_cel->addUniform("c2");
	prog_cel->addUniform("c3");
	prog_cel->addUniform("c4");
	prog_cel->addUniform("thresh");
	prog_cel->setVerbose(false);


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
	shape->loadMesh(RESOURCE_DIR + objFile[0]);
	shape->fitToUnitBox();
	shape->init();

	shape1 = make_shared<Shape>();
	shape1->loadMesh(RESOURCE_DIR + objFile[1]);
	shape1->fitToUnitBox();
	shape1->init();


	
	GLSL::checkError(GET_FILE_LINE);


	
	// for (int i = 0; i < r; ++i) {
	// 	for (int j = 0; j < c; ++j) {
	// 		worldSpaceX[i][j] = j * spacing;
	// 		worldSpaceZ[i][j] = i * spacing;
	// 		printf("(%f, %f) ", worldSpaceX[i][j], worldSpaceZ[i][j]);
	// 	} cout << endl;
	// }

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
	InputManager* im = inputManager; // alias for inputManager bc it's long to type
	
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

	// MV->translate(0, 0, -10);




	camera->applyViewMatrix(MV);
	// MV->translate(0, 0, -10);
	// prog->bind();

	// shape->draw(prog);
	// MV->translate(0, 0, 10);
	// shape->draw(prog);
	// prog->unbind();
	
	if (inputManager->mode == InputManager::Mode::Phong) {

		prog->bind();

		for (int i = 0; i < im->coords->r; ++i) {
			for (int j = 0; j < im->coords->c; ++j) {
				MV->pushMatrix();
				MV->translate(im->coords->worldSpaceX[i][j], 0, im->coords->worldSpaceZ[i][j]);

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
				glUniform3f(prog->getUniform("kd"), im->coords->colors[i][j].x, im->coords->colors[i][j].y, im->coords->colors[i][j].z);
				glUniform3f(prog->getUniform("ks"), im->coords->ks[i][j].x, im->coords->ks[i][j].y, im->coords->ks[i][j].z);
				glUniform1f(prog->getUniform("s"), currS);


				// MV->translate(0, 0, 10);
				// shape->draw(prog);
				// for (int i = 0; i < im->coords->r; ++i) {
				// 	for (int j = 0; j < im->coords->c; ++j) {
				// 		// printf("(%.2f, %.2f) ", im->coords->worldSpaceX[i][j], im->coords->worldSpaceZ[i][j]);
				// 		MV->pushMatrix();
				// 		MV->translate(vec3(0, 0, im->coords->worldSpaceZ[i][j]));
				// 		shape->draw(prog);
				// 		MV->popMatrix();
				// 	}
				// }
				if (int(im->coords->whichShape[i][j]) == 1) {
					shape->draw(prog);
				}
				else {
					shape1->draw(prog);
				}

				MV->popMatrix();
			}
		}
		// glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		// glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		// glUniform3f(prog->getUniform("lightPos1"), lights->lights[0]->pos.x, lights->lights[0]->pos.y, lights->lights[0]->pos.z);
		// glUniform3f(prog->getUniform("lightPos2"), lights->lights[1]->pos.x, lights->lights[1]->pos.y, lights->lights[1]->pos.z);
		// glUniform1f(prog->getUniform("intensity1"), lights->lights[0]->intensity);
		// glUniform1f(prog->getUniform("intensity2"), lights->lights[1]->intensity);

		// MatNode* currMat = mats->getCurrMat();
		// vec3 currKa  = currMat->ka;
		// vec3 currKd  = currMat->kd;
		// vec3 currKs  = currMat->ks;
		// float currS = currMat->s;

		// glUniform3f(prog->getUniform("ka"), currKa.x, currKa.y, currKa.z);
		// glUniform3f(prog->getUniform("kd"), currKd.x, currKd.y, currKd.z);
		// glUniform3f(prog->getUniform("ks"), currKs.x, currKs.y, currKs.z);
		// glUniform1f(prog->getUniform("s"), currS);


		// // MV->translate(0, 0, 10);
		// // shape->draw(prog);
		// // for (int i = 0; i < im->coords->r; ++i) {
		// // 	for (int j = 0; j < im->coords->c; ++j) {
		// // 		// printf("(%.2f, %.2f) ", im->coords->worldSpaceX[i][j], im->coords->worldSpaceZ[i][j]);
		// // 		MV->pushMatrix();
		// // 		MV->translate(vec3(0, 0, im->coords->worldSpaceZ[i][j]));
		// // 		shape->draw(prog);
		// // 		MV->popMatrix();
		// // 	}
		// // }

		// shape->draw(prog);

		//**************************

		// MV->translate(8, 0, -8);

		// glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		// glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		// glUniform3f(prog->getUniform("lightPos1"), lights->lights[0]->pos.x, lights->lights[0]->pos.y, lights->lights[0]->pos.z);
		// glUniform3f(prog->getUniform("lightPos2"), lights->lights[1]->pos.x, lights->lights[1]->pos.y, lights->lights[1]->pos.z);
		// glUniform1f(prog->getUniform("intensity1"), lights->lights[0]->intensity);
		// glUniform1f(prog->getUniform("intensity2"), lights->lights[1]->intensity);

		// // currMat = mats->getCurrMat();
		// // currKa  = currMat->ka;
		// // currKd  = currMat->kd;
		// // currKs  = currMat->ks;
		// // currS = currMat->s;

		// glUniform3f(prog->getUniform("ka"), currKa.x, currKa.y, currKa.z);
		// glUniform3f(prog->getUniform("kd"), currKd.x, currKd.y, currKd.z);
		// glUniform3f(prog->getUniform("ks"), currKs.x, currKs.y, currKs.z);
		// glUniform1f(prog->getUniform("s"), currS);

		// shape->draw(prog);

		//**************************

		prog->unbind();



		
	}
	else if (inputManager->mode == InputManager::Mode::Silhouette) {
		prog_sil->bind();
		glUniformMatrix4fv(prog_sil->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog_sil->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		shape->draw(prog_sil);
		prog_sil->unbind();
	}
	else if (inputManager->mode == InputManager::Mode::Cel) {

		prog_cel->bind();
		glUniformMatrix4fv(prog_cel->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog_cel->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		CelMatNode* currNode = inputManager->celMats->cel_mats.at(inputManager->celMats->currCelMatsIndex);

		// glUniform4fv(prog->getUniform("c1"), 1, glm::value_ptr(currNode->c1));
		// glUniform4fv(prog->getUniform("c2"), 1, glm::value_ptr(currNode->c2));
		// glUniform4fv(prog->getUniform("c3"), 1, glm::value_ptr(currNode->c3));
		// glUniform4fv(prog->getUniform("c4"), 1, glm::value_ptr(currNode->c4));

		
		// currNode->printNode();
		glUniform4f(prog_cel->getUniform("c1"), currNode->c1[0], currNode->c1[1], currNode->c1[2], currNode->c1[3]);
		glUniform4f(prog_cel->getUniform("c2"), currNode->c2[0], currNode->c2[1], currNode->c2[2], currNode->c2[3]);
		glUniform4f(prog_cel->getUniform("c3"), currNode->c3[0], currNode->c3[1], currNode->c3[2], currNode->c3[3]);
		glUniform4f(prog_cel->getUniform("c4"), currNode->c4[0], currNode->c4[1], currNode->c4[2], currNode->c4[3]);
		glUniform3f(prog_cel->getUniform("thresh"), currNode->thresh[0], currNode->thresh[1], currNode->thresh[2]);
		shape->draw(prog_cel);
		prog_cel->unbind();
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

	vec3 nBlue_ka(0.0f, 0.0f, 0.0f);
	vec3 nBlue_kd(0.1f, .15f, 1.0f);
	vec3 nBlue_ks(0.5f, .9f, 0.5f);
	float nBlue_s = 100.0f;
	MatNode* nBlue = new MatNode(nBlue_ka, nBlue_kd, nBlue_ks, nBlue_s);

	vec3 nGray_ka(0.0f, 0.0f, 0.0f);
	vec3 nGray_kd(0.5f, 0.5f, 0.5f);
	vec3 nGray_ks(0.15f, 0.15f, 0.15f);
	float nGray_s = 25.0f;
	MatNode* nGray = new MatNode(nGray_ka, nGray_kd, nGray_ks, nGray_s);

	mats->addMatNode(nWhite);
	mats->addMatNode(nBlue);
	mats->addMatNode(nGray);

	return mats;
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
	srand(time(NULL));
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");

	// string objFile = "";

	// string objFile2 = "";
	string objFile[2];
	if (argc < 3) {
		cout << "Selecting the bunny.obj and cube.obj by default..." << endl;
		cout << "Enter another .obj file name as third argument to select it instead. " << endl;
		objFile[0] = "bunny.obj";
		objFile[1] = "cube.obj";
	}
	else if (argc < 4) {
		objFile[0] = argv[2];
		objFile[1] = "cube.obj";
	}
	else {
		objFile[0] = argv[2];
		objFile[1] = argv[3];
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

	CelMat* celMats = initCelMats();
	
	Coords* coords = new Coords();

	InputManager* inputManager = new InputManager();
	inputManager->lights = lights;
	inputManager->mats = mats;
	inputManager->celMats = celMats;
	inputManager->coords = coords;


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
