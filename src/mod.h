#ifndef _MOD_
#define _MOD_
#include "config.h"

class Mod {
private:
  float power = startPower;
  float powerProc = startPowerProc;
  int powerManagementMode = 0;
public:
  float vcc = VCC;
  unsigned long startMillis = 0;
  unsigned long lastActive = 0;

  void setPower(float setPower) {
    if (powerManagementMode == 1) {
      if (setPower <= maxPower && setPower >=0) {
        power = setPower;
      }
    } else if (powerManagementMode == 0) {
      if (setPower <= 100 && setPower >=0) {
        power = setPower;
      }
    }
  }

  float getPower() {
    return power;
  }

  void setPowerManagementMode(int modeN) {
    powerManagementMode = modeN;
  }

  int getPowerManagementMode() {
    return powerManagementMode;
  }
};
#endif
