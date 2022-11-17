#include <Arduino.h>
#include <Stepper.h>

// https://learn.adafruit.com/adafruit-tb6612-h-bridge-dc-stepper-motor-driver-breakout/using-stepper-motors

// change this to the number of steps on your motor
#define STEPS 200

// #define A2 12
// #define A1 14
// #define A3 27
// #define A4 26

// 0 2 33 32

#define A2 0
#define A1 2
#define A3 33
#define A4 32

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to

// Stepper stepper(STEPS, A4, A3, A2, A1);


Stepper stepper(STEPS, A1, A3, A2, A4); // For Breakout board




void setup()
{
  Serial.begin(115200);
  Serial.println("Stepper test!");

  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(50);
}

void loop()
{
  Serial.println("Forward");
  stepper.step(STEPS);

  delay(3000);

  Serial.println("Backward");
  stepper.step(-STEPS);

  delay(3000);
}