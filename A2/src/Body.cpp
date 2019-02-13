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
  // cout << val << " was pressed with true value of: " << 
  //   char(val) << endl;
  
  switch (val) {
    case Body::KEY_BIG_X: {
      // cout << "upper X" << endl;
      // thetaX+=STEP;
      currentPart->updateRotation(Part::Position::POS_X, STEP, Part::IncDec::Increment);
      break;
    }
    case Body::KEY_BIG_Y: {
      // cout << "upper Y" << endl;
      // thetaY+=STEP;
      currentPart->updateRotation(Part::Position::POS_Y, STEP, Part::IncDec::Increment);
      break;
    }
    case Body::KEY_BIG_Z: {
      // cout << "upper Z" << endl;
      // thetaZ+=STEP;
      currentPart->updateRotation(Part::Position::POS_Z, STEP, Part::IncDec::Increment);
      break;
    }
    case Body::KEY_LIL_X: {
      // cout << "lower X" << endl;
      // thetaX-=STEP;
      currentPart->updateRotation(Part::Position::POS_X, STEP, Part::IncDec::Decrement);
      break;
    }
    case Body::KEY_LIL_Y: { 
      // cout << "lower Y" << endl;
      // thetaY-=STEP;
      currentPart->updateRotation(Part::Position::POS_Y, STEP, Part::IncDec::Decrement);
      break;
    } 
    case Body::KEY_LIL_Z: {
      // cout << "lower Z" << endl;
      // thetaZ-=STEP;
      currentPart->updateRotation(Part::Position::POS_Z, STEP, Part::IncDec::Decrement);
      break;  
    }
    case Body::KEY_PERIOD: {
      // cout << "lower Z" << endl;
      // thetaZ-=STEP;
      std::chrono::system_clock::time_point tock = std::chrono::system_clock::now();
      
      if (delay(tock)) {
        // cout << "nextPart delayed " << endl;
        break;
      }
      cout << "nextPart called " << endl;
      tick = tock;
      nextPart();
      break;
    }   
    case Body::KEY_COMMA: {
      // cout << "lower Z" << endl;
      // thetaZ-=STEP;
      std::chrono::system_clock::time_point tock = std::chrono::system_clock::now();
      
      if (delay(tock)) {
        // cout << "prevPart delayed " << endl;
        break;
      }
      cout << "prevPart called " << endl;
      tick = tock;
      prevPart();
      break;
    }   
    default: {
      // cout << "unrecognized value of: " << val  << " => " << char(val) << endl;   
      break;  
    }                
  }
  
}

void Body::draw(map<string,GLint> unifIDs, int indCount) {
  root->drawPart(&mv, unifIDs, indCount, vec_parts.at(currPartIndex));
}

void Body::setCurrPart(Part* p) {
  if (currentPart != nullptr) {
    currentPart->isCurrent = false;
  }

  currentPart = p;
  currentPart->isCurrent = true;
}

void Body::setCurrPartIndex(int idx) {
  if (idx < 0 || idx >= vec_parts.size()) {
    cerr << "ERROR: setCurrPart -> out of bounds" << endl;
  }

  currentPart = vec_parts.at(idx);
}


void Body::setRoot(Part* p) {
  root = p;
}


void Body::nextPart() {
  // currPartIndex++;
  // if (currPartIndex >= vec_parts.size()) {
  //   currPartIndex = 0;
  // }
  // setCurrPartIndex(currPartIndex);

  // new implementation

  // UNCOMMENT
  deq_parts.push_back(currentPart);
  currentPart = deq_parts.front();
  deq_parts.pop_front();
  for (int i = 0; i < deq_parts.size(); ++i) {
    cout << deq_parts[i]->m_name << " ";
  } cout << endl;

}

void Body::prevPart() {
  // currPartIndex--;
  // if (currPartIndex < 0) {
  //   currPartIndex = vec_parts.size() - 1;
  // }
  // setCurrPartIndex(currPartIndex);

  // new implementation
  deq_parts.push_front(currentPart);
  currentPart = deq_parts.back();
  deq_parts.pop_back();
  for (int i = 0; i < deq_parts.size(); ++i) {
    cout << deq_parts[i]->m_name << " ";
  } cout << endl;
}

bool Body::delay(std::chrono::system_clock::time_point& tock) {
  
  std::chrono::duration<double, std::milli> diff = tock - tick;

  // cout << "tick: " << tick << "tock: " << tock << endl;
  cout << "time taken: " << 
  (std::chrono::duration_cast<std::chrono::milliseconds>(diff)).count()
  << endl;

  cout << "tbuff: " << tbuff << " diff: " << diff.count() << endl;

  if ((std::chrono::duration_cast<std::chrono::milliseconds>(diff)).count() < tbuff) {
    return true;
  }
  else {
    return false;
  }
  
}

void Body::setVectorParts(vector<Part*> v_p) {
  vec_parts = v_p;
}

bool Body::populateDeque() {
  
  // deq_parts.push_front(root);
  populateDeque_helper(currentPart);
  
  deq_parts.pop_front();
  
  return true;
}

bool Body::populateDeque_helper(Part* elem) {

  deq_parts.push_back(elem);
  // cout << "called on : " << elem->m_name << endl;
  for (int i = 0; i < elem->children.size(); ++i) {
    populateDeque_helper(elem->children.at(i));
  }

}


ostream& operator<<(ostream& os, Body& b) {

  return os;
}


