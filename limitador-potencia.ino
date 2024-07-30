#include <Servo.h>
#include <PID_v1.h>

#include "params.hpp"
#include "inc/pwm.hpp"
#include "inc/current.hpp"
#include "inc/voltage.hpp"

// Servo object (calculated PWM output for the ESC)
Servo esc;

// PID's variables
double setpoint;        // Desired value of Power
double input;           // Calculated power
double output;          // PID's output that will be used to set the PWM

// Creating the PID Object
PID pid(&input, &output, &setpoint, KP, KI, KD, DIRECT);

// Global variables used in pwm.cpp for the ISRs
volatile bool flagD2 = false;
volatile bool flagD3 = false;
volatile uint32_t tWidthD2 = 0;
volatile uint32_t tWidthD3 = 0;


void setup(){
  // Start serial output
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Initializing...");

  // attaches the ESC on pinESC to the servo object
  esc.attach(pinESC);

  // setup check pins
  pinMode(pinCheck, OUTPUT);

  // Define the range of PID's outputs (1000 a 2000)
  pid.SetOutputLimits(MIN_PWM, MAX_PWM);

  // Enables the PID controller 
  pid.SetMode(AUTOMATIC);

  // Set pin 2 and 3 to interrupt every time the input changes
  attachInterrupt(digitalPinToInterrupt(2), ISR_D2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(3), ISR_D3, CHANGE);

  Serial.println("Setup done, running loop.");
}


void loop(){
  // PWM signal from the RX and the other Arduino
  // coming from the ISRs
  uint32_t pwmRX = tWidthD2;
  uint32_t pwmArduino = tWidthD3;

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
  if(inRange(outputPWM, pwmArduino, TOLERANCE) & flagD3){
    // PWM is within tolerance, output HIGH on the Check pin
    digitalWrite(pinCheck, HIGH);
  }
  else{
    // PWM is out of range, set check pin to low
    digitalWrite(pinCheck, LOW);
  }

  // send calculated PWM signal to the ESC
  esc.writeMicroseconds(outputPWM);

  // Reset interrupt flags and width
  flagD2 = false;
  flagD3 = false;

  // wait 20 miliseconds (50 Hz)
  delay(20);
}

// Return true if value is close to the reference plus or minus tolerance
bool inRange(uint16_t value, uint16_t reference, uint16_t tolerance){
  return (value <= reference+tolerance && value >= reference-tolerance);
}
