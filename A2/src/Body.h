#ifndef __BODY_H__
#define __BODY_H__

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <chrono>
#include <ratio>
#include <deque>

#define GLEW_STATIC
#include "../../glew-2.1.0/include/GL/glew.h"
#include "../../glfw-3.2.1/include/GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MatrixStack.h"
#include "Part.h"


using std::endl;
using std::cout;
using std::ostream;
using std::string;
using std::map;
using glm::vec3;
using std::deque;





class Body {
private:
  enum Keys { KEY_LIL_X = 120, KEY_LIL_Y = 121, KEY_LIL_Z = 122, KEY_PERIOD = 46, KEY_COMMA = 44,
  KEY_BIG_X = 88, KEY_BIG_Y = 89, KEY_BIG_Z = 90 };
  
  double thetaX = 0, thetaY = 0, thetaZ = 0;

  double CONVERSION = M_PI / 180.0;
  double STEP = 5;

  MatrixStack mv;

  //TODO
  Part* currentPart;

  Part* root;

  deque<Part*> deq_parts;

  void nextPart();
  void prevPart();

  std::chrono::system_clock::time_point tick;

  int DELAY_TIME = 500;
  int64_t tbuff = std::chrono::milliseconds(DELAY_TIME).count();
  // std::chrono::milliseconds time_buffer = 5000ms;
  
  vector<Part*> vec_parts;
  int currPartIndex;

  public:

  Body();
  // void printData() { cout << "data: " << data << endl;}

  void handleKey(int key, int mods);

  void draw(map<string,GLint> unifIDs, int indCount);
  void setCurrPart(Part* p); // deprecated
  void setCurrPartIndex(int indx);
  void setRoot(Part* p);

  friend ostream& operator<<(ostream& os, Body& b);

  void updateCurrentPart(Keys k); // deprecated

  bool delay(std::chrono::system_clock::time_point& t);
  
  void setVectorParts(vector<Part*> l_p);

  bool populateDeque();

  bool populateDeque_helper(Part* elem);

};



#endif