#include <Arduino.h>
#include "../inc/pwm.hpp"

// Global variables used for the ISRs (declared in limitador-potencia.ino)
extern volatile bool flagD2;        // Flags that indicate if the ISR happened
extern volatile bool flagD3;
volatile uint32_t tStartD2 = 0;     // Start time (PWM signal went HIGH)
volatile uint32_t tStartD3 = 0;
extern volatile uint32_t tWidthD2;  // PWM width
extern volatile uint32_t tWidthD3;

// Interrupt handler for pin D2 (Receiver)
// This interrupt runs whenever the state of D2 changes
// from low to high OR high to low
void ISR_D2(){
  // save ISR time
  uint32_t time = micros();

  // set flag
  flagD2 = true;

  // if pin is HIGH, save the start time
  if(digitalRead(2) == HIGH)
    tStartD2 = time;

  // if pin is LOW, calculate the pulse width (current ISR time - start time)
  else
    tWidthD2 = time - tStartD2;
}

// Interrupt handler for pin D3 (second Arduino)
// This interrupt runs whenever the state of D3 changes
// from low to high OR high to low
void ISR_D3(){
  // save ISR time
  uint32_t time = micros();

  // set flag
  flagD3 = true;

  // if pin is HIGH, save the start time
  if(digitalRead(3) == HIGH)
    tStartD3 = time;

  // if pin is LOW, calculate the pulse width (current ISR time - start time)
  else
    tWidthD3 = time - tStartD3;
}
