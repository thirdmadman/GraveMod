#ifndef _BATTERY_
#define _BATTERY_
#include <Arduino.h>
#include "ioadvanced.h"
#include "config.h"

class Battery {
private:
  float voltage = 0;
  IO io;
  Mod *graveMod;
public:

  Battery(Mod *mod) {
    graveMod = mod;
  }

  int getBatteryState(float volts) {

    if (voltage>maxCharchedBattery) {
      //don't let user use this battery
      //return "BATTERY_OVERCHARGED";
      return 1;
    }
    else if ((voltage<=maxCharchedBattery) && (voltage>lowBattery)) {
      //return "BATTERY_NORMAL";
      return 2;
    }
    else if ((voltage<=lowBattery) && (voltage>lowCriticalBattery)) {
      //notice - battery is low
      //return "BATTERY_LOW";
      return 3;
    }
    else if ((voltage<=lowCriticalBattery) && (voltage>(lowCriticalBattery-1))) {
      //don't let user discharge battery more
      //return "BATTERY_LOWCRITICAL";
      return 4;

    }
    else if (voltage<=(lowCriticalBattery-1)) {
      //check your battery - is it there?
      //return "BATTERY_NONE";
      return 5;
    }

    return 0;

  }

  float readBatteryVoltagePrec() {
    voltage = io.readFromPin(measureBattery, 50) * (graveMod.vcc / 1023.0);
    //voltage = io.readFromPin(measureBattery, 50);
    return voltage;
  }

  float readBatteryVoltage() {
    voltage = io.readFromPin(measureBattery, 10) * (graveMod.vcc / 1023.0);
    //voltage = io.readFromPin(measureBattery, 10);
    return voltage;
  }

  float getBatteryVoltage() {
    return voltage;
  }

};

#endif
