#include "Part.h"

Part::Part(string name, vec3 joint_scale, vec3 mesh_scale, vec3 joint_tran, vec3 mesh_tran,  
  Part* p = nullptr, vector<Part*> c = {}, bool show, int iter) : 
  m_name(name), vec3_joint_scale(joint_scale), vec3_mesh_scale(mesh_scale), vec3_joint_translation(joint_tran), 
  vec3_mesh_translation(mesh_tran),  parent(p), children(c), m_show(show), currElem(iter)
{
  // vec3_scale = scale;

}

void Part::drawPart(MatrixStack* mv, map<string, GLint> unifIDs, int indCount, Part* currPart) {

  mv->pushMatrix();
  if (m_show) {
    mv->scale(vec3_joint_scale);
  }

  // transform joint
  mv->translate(vec3_joint_translation);
  
  mv->rotate(thetaX * CONVERSION, vec3_X_AXIS);
  mv->rotate(thetaY * CONVERSION, vec3_Y_AXIS);
  mv->rotate(thetaZ * CONVERSION, vec3_Z_AXIS);

  // mesh translate
    mv->pushMatrix();
      // transform mesh
      if (m_show && (this != currPart)) {
        mv->scale(vec3_mesh_scale);
      }
      else if (m_show && (this == currPart))
      {
        mv->scale(vec3_mesh_scale[0] + .1, vec3_mesh_scale[1] + .1, vec3_mesh_scale[2] + .1);
      }
      mv->translate(vec3_mesh_translation);

      // draw mesh
      if (m_show) {
        glUniformMatrix4fv(unifIDs["MV"], 1, GL_FALSE, value_ptr(mv->topMatrix()));
        glDrawArrays(GL_TRIANGLES, 0, indCount);
      }

    mv->popMatrix();

  // draw children
  for (int i = 0; i < children.size(); ++i) {
    children.at(i)->drawPart(mv, unifIDs, indCount, currPart);
  }

  // does not affect children
  if (!m_show) {
    mv->scale(0, 0, 0);
  }

  mv->popMatrix();
}

void Part::updateRotation(Position p, double step, IncDec incdec) {
  switch (p) {
    case Part::POS_X:
      thetaX = thetaX + incdec * step;
      break;
    case Part::POS_Y:
      thetaY = thetaY + incdec * step;
      break;
    case Part::POS_Z:
      thetaZ = thetaZ + incdec * step; 
      break;
    default: 
      cout << "Error in Part::updateRotation(), position not recognized" << endl;
      break;
  }
}

void Part::setChildren(vector<Part*> c) {
  
  children = c;
}

bool Part::hasNextChild() {
  if ((currElem + 1) >= children.size()) {
    return false;
  }
  else {
    return true;
  }
}

bool Part::hasPrevChild() {
  if ((currElem - 1) < 0) {
    return false;
  }
  else {
    return true;
  }
}

void Part::resetChildIter() {
  currElem = - 1;
}

Part* Part::goParent() {

  cout << "goParent() called on: " << m_name << endl;
  if (parent == nullptr) {
    cout << "PARENT IS NULL" << endl;
    return this;
  }
  return parent;
}

Part* Part::goNextChild() {
  currElem++;
  return children.at(currElem);
}

Part* Part::goPrevChild() {
  currElem--;
  return children.at(currElem);

}