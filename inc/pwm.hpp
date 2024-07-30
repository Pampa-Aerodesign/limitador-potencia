#pragma once
#include "../params.hpp"

// Interrupt handler for pin D2 (Receiver)
// This interrupt runs whenever the state of D2 changes
// from low to high OR high to low
void ISR_D2();

// Interrupt handler for pin D3 (second Arduino)
// This interrupt runs whenever the state of D3 changes
// from low to high OR high to low
void ISR_D3();
