#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <cmath>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <chrono>

using glm::vec3;
using std::vector;
using std::endl;
using std::cout;

struct MatNode {

  MatNode(vec3 kamb, vec3 kdif, vec3 kspec, float spec) :
    ka(kamb), kd(kdif), ks(kspec), s(spec) {};
  vec3 ka;
  vec3 kd;
  vec3 ks;
  float s;
};

class Material {
  int KEY_M = 77;

  vector<MatNode*> materials = {};
  int currMatIndex = 0;

  std::chrono::system_clock::time_point tick;

  int DELAY_TIME = 500;
  int64_t tbuff = std::chrono::milliseconds(DELAY_TIME).count();

  bool delay(std::chrono::system_clock::time_point& tock);

  public:

  MatNode* getCurrMat();

  void addMatNode(MatNode* n) { materials.push_back(n); };

  void handleKey(int key, int mods);

  void nextMat();
  void prevMat();

};


#endif