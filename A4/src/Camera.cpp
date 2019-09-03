#define _USE_MATH_DEFINES
#include <cmath> 
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "MatrixStack.h"

Camera::Camera() :
	aspect(1.0f),
	fovy((float)(45.0*M_PI/180.0)),
	znear(0.1f),
	zfar(1000.0f),
	rotations(0.0, 0.0),
	translations(0.0f, 0.0f, -5.0f),
	rfactor(0.01f),
	tfactor(0.001f),
	sfactor(0.005f),
	position(0.0f, -0.5f, 0.0f)
{
}

Camera::~Camera()
{
}

void Camera::mouseClicked(float x, float y, bool shift, bool ctrl, bool alt)
{
	mousePrev.x = x;
	mousePrev.y = y;
	if(shift) {
		state = Camera::TRANSLATE;
	} else if(ctrl) {
		state = Camera::SCALE;
	} else {
		state = Camera::ROTATE;
	}
}

void Camera::mouseMoved(float x, float y)
{
	glm::vec2 mouseCurr(x, y);
	glm::vec2 dv = mouseCurr - mousePrev;
	switch(state) {
		case Camera::ROTATE:
			rotations += rfactor * dv;
			if (rotations.y < -MAX_PITCH) {
				rotations.y = -MAX_PITCH;
			}
			else if (rotations.y > MAX_PITCH) {
				rotations.y = MAX_PITCH;
			}
			break;
		case Camera::TRANSLATE:
			translations.x -= translations.z * tfactor * dv.x;
			translations.y += translations.z * tfactor * dv.y;
			break;
		case Camera::SCALE:
			translations.z *= (1.0f - sfactor * dv.y);
			break;
	}
	mousePrev = mouseCurr;

}

void Camera::keyPressed(int key, int mods) {
	// FIXME
	vec3 F_yaw = normalize(vec3(sin(rotations.x), 0, cos(rotations.x)));
	vec3 R_yaw = normalize(cross(vec3(0, 1, 0), F_yaw));

	if (key == KEY_W_MAIN) {
		// position.x += cos(rotations.x);
		// position.y += rotations.y;
		// position.x -= STEP_SIZE;
		position = vec3(position.x - F_yaw.x * STEP_SIZE, position.y + F_yaw.y * STEP_SIZE, position.z + F_yaw.z * STEP_SIZE);
	}
	else if (key == KEY_S_MAIN) {
		// position.x += STEP_SIZE;
		position = vec3(position.x + F_yaw.x * STEP_SIZE, position.y + F_yaw.y * STEP_SIZE, position.z - F_yaw.z * STEP_SIZE);
	}
	else if (key == KEY_A_MAIN) {
		position = vec3(position.x + R_yaw.x * STEP_SIZE, position.y + R_yaw.y * STEP_SIZE, position.z - R_yaw.z * STEP_SIZE);
	}
	else if (key == KEY_D_MAIN) {
		position = vec3(position.x - R_yaw.x * STEP_SIZE, position.y + R_yaw.y * STEP_SIZE, position.z + R_yaw.z * STEP_SIZE);
	}
}

void Camera::applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const
{
	// Modify provided MatrixStack
	P->multMatrix(glm::perspective(fovy, aspect, znear, zfar));
}

void Camera::applyViewMatrix(std::shared_ptr<MatrixStack> MV) const
{
	MV->translate(translations);
	MV->rotate(rotations.y, glm::vec3(1.0f, 0.0f, 0.0f));
	MV->rotate(rotations.x, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::applyViewMatrixFreeLook(std::shared_ptr<MatrixStack> MV) const
{
	// MV->translate(translations);
	glm::vec2 f = glm::normalize(glm::vec2(cos(rotations.x), sin(rotations.y)));

	MV->rotate(rotations.y, glm::vec3(1.0f, 0.0f, 0.0f));
	MV->rotate(rotations.x, glm::vec3(0.0f, 1.0f, 0.0f));
	MV->translate(position);
}
