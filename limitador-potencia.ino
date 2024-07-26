#include <Servo.h>
#include <PID_v1.h>

#include "params.hpp"
#include "inc/pwm.hpp"
#include "inc/current.hpp"
#include "inc/voltage.hpp"

// Input PWM channels
Channels_t Channels[NUM_CHANNELS] = {0};

// Servo object (calculated PWM output for the ESC)
Servo esc;

// PID's variables
double setpoint;        // Desired value of Power
double input;           // Calculated power
double output;          // PID's output that will be used to set the PWM

// Initial PID's Parameters
double kp = 2.0, ki = 5.0, kd = 1.0;

// Creating the PID Object
PID pid(&input, &output, &setpoint, kp, ki, kd, DIRECT);


void setup(){
  // DEBUG
  // Serial.begin(115200);

  esc.attach(pinESC);  // attaches the ESC on pinESC to the servo object

  // setup check pins
  pinMode(pinCheck, OUTPUT);
  digitalWrite(pinCheck, HIGH);

  // setup channel pins
  for(int i = 0; i < NUM_CHANNELS; i++){
    Channels[i].pin = PinsPWM[i];
    pinMode(Channels[i].pin, INPUT_PULLUP);
    Channels[i].pinStateLast = digitalRead(Channels[i].pin);
    Channels[i].tStart = micros();
  }
  // Define the range of PID's outputs (1000 a 2000)
  pid.SetOutputLimits(1000, 2000);

  // Enables the PID controller 
  pid.SetMode(AUTOMATIC);
}

void loop(){
  // read all PWM channels
  readPWMChannels(Channels);

  // PWM signal coming from the other Arduino and from the RX
  uint16_t pwmArduino = Channels[0].tWidth;
  uint16_t pwmRX = Channels[1].tWidth;

  // calculate limited PWM
  // this is where we read voltage and current from the battery
  // and calculate a new PWM so that power (volts * amps)
  // doesn't exceed a specified value
  uint16_t outputPWM;

  // reading of current
  // due to the library this data needs to be a Double
  double current = readCurrent();

  // reading of voltage
  // due to the library this data needs to be a Double
  double voltage = readVoltage();

  // calculation of power: (current * voltage)
  // due to the library this data needs to be a Double
  double power = voltage * current;

  // run the PID calculation
  pid.Compute();

  // PID controller is used to generate a new PWM
  // uses a cast to guarantee 16-bit integer format
  outputPWM = static_cast<uint16_t>(output);

  // check if calculated PWM is close to the PWM calculated by the other arduino
  if(inRange(outputPWM, pwmArduino, TOLERANCE)){
    // PWM is within tolerance, output HIGH on the Check pin
    digitalWrite(pinCheck, HIGH);
  }
  else{
    // PWM is out of range, set check pin to low
    digitalWrite(pinCheck, LOW);
  }

  // send calculated PWM signal to the ESC
  esc.writeMicroseconds(outputPWM); 
}


// Return true if value is close to the reference plus or minus tolerance
bool inRange(uint16_t value, uint16_t reference, uint16_t tolerance){
  return (value <= reference+tolerance && value >= reference-tolerance);
}
