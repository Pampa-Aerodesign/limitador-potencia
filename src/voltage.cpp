#include <Arduino.h>
#include "../inc/voltage.hpp"
#include "../params.hpp"

// This function will read the voltage sensor and return its value
double readVoltage(){
  double voltage;

  // read voltage sensor
  double value = analogRead(pinVoltage);
  value = (value * VREF) / 1024.0; // VREF might change depending on power supply
  voltage = value / (R2/(R1+R2));

  return voltage;
}