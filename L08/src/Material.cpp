#include "Material.h"


void Material::nextMat() {
  currMatIndex = (currMatIndex + 1 >= materials.size()) ? 0 : currMatIndex + 1;

}

void Material::prevMat() {
  currMatIndex = (currMatIndex - 1 < 0) ? materials.size() - 1 : currMatIndex - 1;
}

MatNode* Material::getCurrMat() {
  return materials.at(currMatIndex);
}

void Material::handleKey(int key, int mods) {

  std::chrono::system_clock::time_point tock = std::chrono::system_clock::now();

  if (delay(tock)) {
    return;
  }
  tick = tock;

	if (key == KEY_M && !mods) {
    nextMat();
	}
	else if (key == KEY_M && mods) {
    prevMat();
	}
}

bool Material::delay(std::chrono::system_clock::time_point& tock) {
  
  std::chrono::duration<double, std::milli> diff = tock - tick;

  if ((std::chrono::duration_cast<std::chrono::milliseconds>(diff)).count() < tbuff) {
    return true;
  }
  else {
    return false;
  }
  
}