#include <Servo.h>

// number of PWM inputs
#define NUM_CHANNELS 2

#define MAXPWM 2000

// Check pin input and output
const uint8_t pinCheckOut = 13;
const uint8_t pinCheckIn = 12;

// PWM input pins
const uint8_t pinPWMArduino = 10;
const uint8_t pinPWMRX = 11;

int maxFlag = 0;

const uint8_t pinsPWM[] = {
  pinPWMArduino,
  pinPWMRX
};

// Channel struct
typedef struct structChannels{
  uint8_t   pin;            // Channel pin
  uint8_t   pinStateLast;   // Last state the pin was in (HIGH or LOW)
  uint32_t  tStart;         // Time in microseconds the pin changed to HIGH
  uint32_t  tWidth;         // Width of the pulse in microseconds
} sChannels_t;

sChannels_t Channels[NUM_CHANNELS] = {0};

// Servo object (calculated PWM output for the ESC)
Servo esc;

void setup(){
  // Serial.begin(115200);

  esc.attach(9);  // attaches the ESC on pin 9 to the servo object

  // setup check pins
  pinMode(pinCheckOut, OUTPUT);
  pinMode(pinCheckIn, INPUT);
  digitalWrite(13, HIGH);

  // setup channel pins
  for(int i = 0; i < NUM_CHANNELS; i++){
    Channels[i].pin = pinsPWM[i];
    pinMode(Channels[i].pin, INPUT_PULLUP);
    Channels[i].pinStateLast = digitalRead(Channels[i].pin);
    Channels[i].tStart = micros();
  }
}

void loop(){
  // read PWM channels
  PWMRead();

  // calculate limited PWM
  // this is where we read voltage and current from the battery
  // and calculate a new PWM
  // so that power (volts * amps) doesn't exceed a specified value
  uint16_t pwm = 1200; // placeholder

  // generate calculated PWM servo signal
  esc.writeMicroseconds(pwm); 

  // PWM signal coming from the other Arduino and from the RX
  uint16_t pwmArduino = Channels[0].tWidth;
  uint16_t pwmRX = Channels[1].tWidth;

  // read check pin
  uint8_t check = digitalRead(pinCheckIn);

  // check if calculated PWM is close to the PWM calculated by the other arduino
  if(inRange(pwm, pwmArduino, 100)){
    // PWM is within range, output HIGH on the Check pin
    digitalWrite(pinCheckOut, HIGH);
  }
  else{
    // PWM is out of range, output negated Check pin from the other Arduino
    digitalWrite(pinCheckOut, !check);
  }
}

// Return true if value is close to the reference plus or minus tolerance
bool inRange(uint16_t value, uint16_t reference, uint16_t tolerance){
  return (value <= reference+tolerance && value >= reference-tolerance);
}

// This function will read all PWM channels
// and store the pulse width in microseconds
void PWMRead(){
  // Current channel
  static uint8_t channel = 0;

  // Current time in microseconds
  uint32_t timeNow = micros();

  // each pass we check one channel
  // read the state of the input
  uint8_t pinStateNow = digitalRead(Channels[channel].pin);

  if((timeNow - Channels[channel].tStart) > MAXPWM){
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

        // map tWidth to 0-180 (this function is not used so it was commented out)
        //Channels[channel].degrees = (uint8_t) map(Channels[channel].tWidth, 1100, 1900, 0, 180);

        // print channel PWM width
        // if(channel == 0 && millis()%500 < 20){
        //   Serial.print("\tCH:");     
        //   Serial.print(channel+1);
        //   Serial.print(" ");
        //   Serial.println(Channels[channel].tWidth);
        // }
      }
    }
  }

  if(pinStateNow == LOW){
    Channels[channel].pinStateLast = LOW;
    Channels[channel].tStart = timeNow;
    maxFlag = 0;
  }

  // go to next channel
  if(++channel >= NUM_CHANNELS)
    channel = 0;
}
