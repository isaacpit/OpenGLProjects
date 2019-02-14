#include "Body.h"

Body::Body() {
  mv = MatrixStack();
  tick = {};
  currentPart = nullptr;
  root = nullptr;
  currPartIndex = 0;
  deq_parts = {};
}

void Body::handleKey(int key, int mods) {
  int val = key + ! mods * 32;
  
  switch (val) {
    case Body::KEY_BIG_X: {
      currentPart->updateRotation(Part::Position::POS_X, STEP, Part::IncDec::Increment);
      break;
    }
    case Body::KEY_BIG_Y: {
      currentPart->updateRotation(Part::Position::POS_Y, STEP, Part::IncDec::Increment);
      break;
    }
    case Body::KEY_BIG_Z: {
      currentPart->updateRotation(Part::Position::POS_Z, STEP, Part::IncDec::Increment);
      break;
    }
    case Body::KEY_LIL_X: {
      currentPart->updateRotation(Part::Position::POS_X, STEP, Part::IncDec::Decrement);
      break;
    }
    case Body::KEY_LIL_Y: { 

      currentPart->updateRotation(Part::Position::POS_Y, STEP, Part::IncDec::Decrement);
      break;
    } 
    case Body::KEY_LIL_Z: {
      currentPart->updateRotation(Part::Position::POS_Z, STEP, Part::IncDec::Decrement);
      break;  
    }
    case Body::KEY_PERIOD: {
      std::chrono::system_clock::time_point tock = std::chrono::system_clock::now();
      
      if (delay(tock)) {
        break;
      }
      cout << "nextPart called " << endl;
      tick = tock;
      nextPart();
      break;
    }   
    case Body::KEY_COMMA: {
      std::chrono::system_clock::time_point tock = std::chrono::system_clock::now();
      
      if (delay(tock)) {
        break;
      }
      cout << "prevPart called " << endl;
      tick = tock;
      prevPart();
      break;
    }   
    default: {
      break;  
    }                
  }
  
}

void Body::draw(map<string,GLint> unifIDs, int indCount) {
  root->drawPart(&mv, unifIDs, indCount, currentPart);
}

void Body::setCurrPart(Part* p) {
  if (currentPart != nullptr) {
    currentPart->isCurrent = false;
  }

  currentPart = p;
  currentPart->isCurrent = true;
}


void Body::setRoot(Part* p) {
  root = p;
}


void Body::nextPart() {

  deq_parts.push_back(currentPart);
  currentPart = deq_parts.front();
  deq_parts.pop_front();
  for (int i = 0; i < deq_parts.size(); ++i) {
    cout << deq_parts[i]->m_name << " ";
  } cout << endl;

}

void Body::prevPart() {
  deq_parts.push_front(currentPart);
  currentPart = deq_parts.back();
  deq_parts.pop_back();
  for (int i = 0; i < deq_parts.size(); ++i) {
    cout << deq_parts[i]->m_name << " ";
  } cout << endl;
}

bool Body::delay(std::chrono::system_clock::time_point& tock) {
  
  std::chrono::duration<double, std::milli> diff = tock - tick;

  if ((std::chrono::duration_cast<std::chrono::milliseconds>(diff)).count() < tbuff) {
    return true;
  }
  else {
    return false;
  }
  
}

bool Body::populateDeque() {
  
  populateDeque_helper(currentPart);
  
  deq_parts.pop_front();
  
  return true;
}

bool Body::populateDeque_helper(Part* elem) {

  deq_parts.push_back(elem);

  for (int i = 0; i < elem->children.size(); ++i) {
    populateDeque_helper(elem->children.at(i));
  }
  return true;
}


ostream& operator<<(ostream& os, Body& b) {

  return os;
}


