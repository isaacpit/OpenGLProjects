#ifndef __LIGHT_H__
#define __LIGHT_H__

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

struct LightNode {
  LightNode(vec3 v, float f) : pos(v), intensity(f) {};
  vec3 pos;
  float intensity;

};

class Light {
  int KEY_L = 76;
  int KEY_X = 88;
  int KEY_Y = 89;

  int POS_X = 0;
  int POS_Y = 1;




  std::chrono::system_clock::time_point tick;

  int DELAY_TIME = 250;
  int64_t tbuff = std::chrono::milliseconds(DELAY_TIME).count();

  float step_size = .5;

  bool delay(std::chrono::system_clock::time_point& tock);

  void updateCurrent(int pos, int mods);

  public:
  int currLightIndex = 0;
  vector<LightNode *> lights;

  void addLightNode(LightNode* l) { lights.push_back(l); };

  void nextLight();
  void prevLight();

  LightNode* getCurrLight();

  void handleKey(int key, int mods);

  void printLights() {
    for (int i = 0; i < lights.size(); ++i) {
      cout << i << " | (" << lights.at(i)->pos.x << ", " << 
      lights.at(i)->pos.y << ", " << 
      lights.at(i)->pos.z << ") | intensity: " << lights.at(i)->intensity << endl;
    }
    
  }


};



#endif 