#include <Arduino.h>
#include "../inc/voltage.hpp"
#include "../params.hpp"

// This function will read the voltage sensor and return its value
double readVoltage(){
  double voltage;

  // read voltage sensor
  int adcValue = analogRead(pinVoltage);

  // apply correction factor
  voltage = (mVpS * adcValue) / VOLT_FACTOR;

  return voltage;
}