#include "Body.h"

void Body::handleKey(int key, int mods) {
  int val = key + ! mods * 32;
  // cout << val << " was pressed with true value of: " << 
  //   char(val) << endl;

  switch (val) {
    case Body::KEY_BIG_X: 
      cout << "upper X" << endl;
      thetaX+=STEP;
      break;
    case Body::KEY_BIG_Y: 
      cout << "upper Y" << endl;
      thetaY+=STEP;
      break;
    case Body::KEY_BIG_Z: 
      cout << "upper Z" << endl;
      thetaZ+=STEP;
      break;
    case Body::KEY_LIL_X: 
      cout << "lower X" << endl;
      thetaX-=STEP;
      break;
    case Body::KEY_LIL_Y: 
      cout << "lower Y" << endl;
      thetaY-=STEP;
      break;
    case Body::KEY_LIL_Z: 
      cout << "lower Z" << endl;
      thetaZ-=STEP;
      break;   
    default: 
      cout << "unrecognized value of: " << val  << " => " << char(val) << endl;   
      break;                  
  }
  
}

MatrixStack Body::draw(map<string,GLint> unifIDs, int indCount) {

  mv.pushMatrix();

    mv.translate(0, 0, -10);
    mv.rotate(thetaX * CONVERSION, vec3(1, 0, 0));
    mv.rotate(thetaY * CONVERSION, vec3(0, 1, 0));
    mv.rotate(thetaZ * CONVERSION, vec3(0, 0, 1));

    mv.pushMatrix();
      mv.translate(3, 0, 0);
      glUniformMatrix4fv(unifIDs["MV"], 1, GL_FALSE, value_ptr(mv.topMatrix()));
      glDrawArrays(GL_TRIANGLES, 0, indCount);

    mv.popMatrix();

    glUniformMatrix4fv(unifIDs["MV"], 1, GL_FALSE, value_ptr(mv.topMatrix()));
    glDrawArrays(GL_TRIANGLES, 0, indCount);

  mv.popMatrix();
  return mv;
}

ostream& operator<<(ostream& os, Body& b) {

  return os;
}

