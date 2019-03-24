#pragma  once
#ifndef __Camera__
#define __Camera__

#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class MatrixStack;

using std::endl;
using std::cout;
using glm::vec2;
using glm::vec3;
using glm::normalize;
using glm::cross;

class Camera
{
public:
	enum {
		ROTATE = 0,
		TRANSLATE,
		SCALE
	};


	int KEY_M_MAIN = 77;
	int KEY_L_MAIN = 76;
	int KEY_X_MAIN = 88;
	int KEY_Y_MAIN = 89;
	int KEY_W_MAIN = 87;
	int KEY_A_MAIN = 65;
	int KEY_S_MAIN = 83;
	int KEY_D_MAIN = 68;
	
	Camera();
	virtual ~Camera();
	void setInitDistance(float z) { translations.z = -std::abs(z); }
	void setAspect(float a) { aspect = a; };
	void setRotationFactor(float f) { rfactor = f; };
	void setTranslationFactor(float f) { tfactor = f; };
	void setScaleFactor(float f) { sfactor = f; };
	void mouseClicked(float x, float y, bool shift, bool ctrl, bool alt);
	void mouseMoved(float x, float y);
	void applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const;
	void applyViewMatrix(std::shared_ptr<MatrixStack> MV) const;
	void applyViewMatrixFreeLook(std::shared_ptr<MatrixStack> MV) const;
	void keyPressed(int, int);
	
private:
	float aspect;
	float fovy;
	float znear;
	float zfar;
	glm::vec2 rotations;
	glm::vec3 translations;
	glm::vec2 mousePrev;
	int state;
	float rfactor;
	float tfactor;
	float sfactor;

	glm::vec3 position;

	float STEP_SIZE = 0.25f;
	float MAX_PITCH = M_PI / 3.0f;
};

#endif
