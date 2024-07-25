#include <Arduino.h>
#include "../inc/current.hpp"
#include "../params.hpp"

// This function will read the current sensor and return its value
double readCurrent(){
  double current;

  // read current sensor
  double adcValue = analogRead(pinCurrent);

  adcValue = resolution * adcValue;
  current = adcValue / sens;
  // current = (adcValue - offset) / sens;

  return current;
}
