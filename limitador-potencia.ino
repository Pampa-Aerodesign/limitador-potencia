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

// Creating the PID Object
PID pid(&input, &output, &setpoint, KP, KI, KD, DIRECT);


void setup(){
  // DEBUG
  // Serial.begin(115200);

  // attaches the ESC on pinESC to the servo object
  esc.attach(pinESC); 

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
  pid.SetOutputLimits(MIN_PWM, MAX_PWM);

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

  // map input PWM to output power
  // throttle at   0% =   0 watts
  // throttle at 100% = 700 watts
  int targetPower = map(pwmRX, MIN_PWM, MAX_PWM, 0, MAX_POWER);

  // set input and setpoint
  setpoint = targetPower;
  input = power;

  // run the PID calculation
  pid.Compute();

  // output the new PWM from the PID
  // uses a cast to guarantee 16-bit integer format
  outputPWM = static_cast<uint16_t>(output);

  // check if calculated PWM is close to the PWM calculated by the other arduino
  if(inRange(outputPWM, pwmArduino, TOLERANCE)){
    // PWM is within tolerance, output HIGH on the Check pin
    digitalWrite(pinCheck, HIGH);

    // send calculated PWM signal to the ESC
    esc.writeMicroseconds(outputPWM); 
  }
  else{
    // PWM is out of range, set check pin to low
    digitalWrite(pinCheck, LOW);

    // send minimum PWM signal to ESC
    esc.writeMicroseconds(MIN_PWM);
  }
}

// Return true if value is close to the reference plus or minus tolerance
bool inRange(uint16_t value, uint16_t reference, uint16_t tolerance){
  return (value <= reference+tolerance && value >= reference-tolerance);
}
