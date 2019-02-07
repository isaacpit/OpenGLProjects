#ifndef __BODY_H__
#define __BODY_H__

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include<cmath>

#define GLEW_STATIC
#include "../../glew-2.1.0/include/GL/glew.h"
#include "../../glfw-3.2.1/include/GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MatrixStack.h"


using std::endl;
using std::cout;
using std::ostream;
using std::string;
using std::map;
using glm::vec3;



class Body {
private:
  enum Keys { KEY_LIL_X = 120, KEY_LIL_Y = 121, KEY_LIL_Z = 122, KEY_PERIOD = 46, KEY_COMMA = 44,
  KEY_BIG_X = 88, KEY_BIG_Y = 89, KEY_BIG_Z = 90 };
  
  double thetaX = 0, thetaY = 0, thetaZ = 0;

  double CONVERSION = M_PI / 180.0;
  double STEP = 5;

  MatrixStack mv;

  public:
  // void printData() { cout << "data: " << data << endl;}

  void handleKey(int key, int mods);

  MatrixStack draw(map<string,GLint> unifIDs, int indCount);

  friend ostream& operator<<(ostream& os, Body& b);
};



#endif