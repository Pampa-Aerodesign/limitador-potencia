#include <Arduino.h>
#include "../inc/current.hpp"
#include "../params.hpp"

// This function will read the current sensor and return its value
double readCurrent(){
  double current;

  // read current sensor
  int adcValue = analogRead(pinCurrent);

  // apply correction factor
  current = (mVpS * adcValue) / AMP_FACTOR;

  return current;
}
