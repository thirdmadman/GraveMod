#ifndef _IOADVANCED_
#define _IOADVANCED_
#include <Arduino.h>
#include "ioadvanced.h"
class IO {
public:

  float readFromPin(int pinNumber, int arryLenth) {
    float read[arryLenth];
    for (int i = 0; i < arryLenth - 1; i++) {
      read[i] = analogRead(pinNumber);
    }

    for (int i = 0; i < arryLenth - 1; i++) {
      for (int j = 0; j < arryLenth - i - 1; j++) {
        if (read[j] > read[j + 1]) {
          float temp = read[j];
          read[j] = read[j + 1];
          read[j + 1] = temp;
        }
      }
    }

    float outFloat = 0;
    int spector = 6;
    for (int i = (((arryLenth/2)-(spector/2))-1); i < (((arryLenth/2)+(spector/2))-1); i++) {
      outFloat += read[i];
    }
    outFloat /= spector;
    return outFloat;
  }
  
};

#endif
