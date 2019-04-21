#pragma once
#ifndef _SHAPESKIN_H_
#define _SHAPESKIN_H_

#include <memory>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MatrixStack.h"

class MatrixStack;
class Program;

class ShapeSkin
{
public:
	int nbones, nframes, nverts;

	ShapeSkin();
	virtual ~ShapeSkin();
	void loadMesh(const std::string &meshName);
	void loadAttachment(const std::string &filename);
	void loadSkeleton(const std::string &filename);
	void setProgram(std::shared_ptr<Program> p) { prog = p; }
	void init();
	void draw() const;
	void drawBindPoseFrenetFrames(std::shared_ptr<MatrixStack> MV, float t, bool debug = false, float len = .05);

	void drawAnimationFrenetFrames(std::shared_ptr<MatrixStack> MV, float t, bool debug = false, float len = .05);
	
private:
	std::shared_ptr<Program> prog;
	std::vector<unsigned int> elemBuf;
	std::vector<float> posBuf;
	std::vector<float> origPosBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	std::vector<float> weightBuf;
	unsigned elemBufID;
	unsigned posBufID;
	unsigned origPosBufID;
	unsigned norBufID;
	unsigned texBufID;

	std::vector<std::vector<glm::mat4>> vecTransforms;
	std::vector<glm::mat4> bindPoseNoInverse;
	std::vector<glm::mat4> bindPoseInverse;

	void drawPoint(std::shared_ptr<MatrixStack> MV, glm::mat4 E, float t , bool debug, float len);

	
	
};

#endif
