#ifndef __PART_H__
#define __PART_H__

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

using std::vector;
using std::endl;
using std::cout;
using std::ostream;
using std::string;
using std::map;
using glm::vec3;
using std::cerr;

class Part {
  private:

    vec3 vec3_X_AXIS = vec3(1, 0, 0);
    vec3 vec3_Y_AXIS = vec3(0, 1, 0);
    vec3 vec3_Z_AXIS = vec3(0, 0, 1);
    
    // MatrixStack* mv;

    double thetaX = 0;
    double thetaY = 0;
    double thetaZ = 0;

    double CONVERSION = M_PI / 180.0;

    // vec3 vec3_scale;
    vec3 vec3_joint_scale;
    vec3 vec3_mesh_scale;

    vec3 vec3_joint_translation;
    vec3 vec3_mesh_translation;

    int currElem;
    
    Part* parent;
    
    int m_show;

  public:
    vector<Part*> children;

    string m_name;

    enum Position {
      POS_X, POS_Y, POS_Z
    };

    enum IncDec {
      Increment = 1, Decrement = -1
    };

    bool isCurrent;

    Part(string name, 
          vec3 joint_scale, 
          vec3 mesh_scale, 
          vec3 joint_tran, 
          vec3 mesh_tran, 
          Part* p, 
          vector<Part*> c, 
          bool bool_show = true, 
          int iter = -1);

    void drawPart(MatrixStack* mv, map<string, GLint> unifIDs, int indCnt, Part* currPart);

    void updateRotation(Position p, double step, IncDec incdec);

    void setChildren(vector<Part*> c);

    bool hasNextChild();

    bool hasPrevChild();

    void resetChildIter();

    Part* goParent();

    Part* goNextChild();

    Part* goPrevChild();

};






#endif 