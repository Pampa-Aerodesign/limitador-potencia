#include <Arduino.h>
#include "../inc/pwm.hpp"

// Flag used to check if PWM reached MAX_PWM_OUTPUT
int maxFlag = 0;

// This function will read all PWM channels
// and store the pulse width in microseconds
void readPWMChannels(Channels_t Channels[]){
  // Current channel
  static uint8_t channel = 0;

  // Current time in microseconds
  uint32_t timeNow = micros();

  // each pass we check one channel
  // read the state of the input
  uint8_t pinStateNow = digitalRead(Channels[channel].pin);

  // if PWM is above MAX_PWM_OUTPUT, set maxFlag and output 900 (throttle cut)
  if((timeNow - Channels[channel].tStart) > MAX_PWM_OUTPUT){
    Channels[channel].tWidth = 900;
    maxFlag = 1;
  }

  // if pin state changed (different than last)
  if(maxFlag == 0){
    if(pinStateNow != Channels[channel].pinStateLast){        
      // save as new last
      Channels[channel].pinStateLast = pinStateNow;

      if(pinStateNow == HIGH){            
        // pin changed from low to high; log the current time
        Channels[channel].tStart = timeNow;
      }
      else{
        // pin changed from high to low; current time minus start time is the pulse width
        Channels[channel].tWidth = timeNow - Channels[channel].tStart;
      }
    }
  }
  // else?
  if(pinStateNow == LOW){
    Channels[channel].pinStateLast = LOW;
    Channels[channel].tStart = timeNow;
    maxFlag = 0;
  }

  // go to next channel
  if(++channel >= NUM_CHANNELS)
    channel = 0;
}
