#include "Light.h"

void Light::nextLight() {
  currLightIndex = (currLightIndex + 1 >= lights.size()) ? 0 : currLightIndex + 1;

}

void Light::prevLight() {
  currLightIndex = (currLightIndex - 1 < 0) ? lights.size() - 1 : currLightIndex - 1;
}

LightNode* Light::getCurrLight() {
  return lights.at(currLightIndex);
}

void Light::updateCurrent(int pos, int mods) {
  LightNode* curr = lights.at(currLightIndex);
  printf("(%f, %f, %f) BEFORE\n", curr->pos.x, curr->pos.y, curr->pos.z);

  curr->pos[pos] = (mods) ? curr->pos[pos] - step_size : curr->pos[pos] + step_size;
  
  printf("(%f, %f, %f) NEW POS  BEFORE \n", curr->pos.x, curr->pos.y, curr->pos.z);

}

void Light::handleKey(int key, int mods) {




  if (key == KEY_X) {
    updateCurrent(POS_X, mods);
  }
  if (key == KEY_Y) {
    updateCurrent(POS_Y, mods);
  }
  if (key == KEY_L) {

    // Delay switching between lights because it is too fast to 
    // accurately switch easily. However I personally 
    // like updating the actual positions quickly so I only 
    // delay for switching cameras.
    std::chrono::system_clock::time_point tock = std::chrono::system_clock::now();

    if (delay(tock)) {
      return;
    }

    tick = tock;
    if (!mods) { nextLight(); } 
    else { prevLight(); }
  }

}

bool Light::delay(std::chrono::system_clock::time_point& tock) {
  
  std::chrono::duration<double, std::milli> diff = tock - tick;

  if ((std::chrono::duration_cast<std::chrono::milliseconds>(diff)).count() < tbuff) {
    return true;
  }
  else {
    return false;
  }
  
}

