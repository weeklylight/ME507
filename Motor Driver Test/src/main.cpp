#include <Arduino.h>
#include <Stepper.h>

// https://learn.adafruit.com/adafruit-tb6612-h-bridge-dc-stepper-motor-driver-breakout/using-stepper-motors

// change this to the number of steps on your motor
#define STEPS 150

// #define AIN2 12
// #define AIN1 14
// #define BIN1 27
// #define BIN2 26

// 0 2 33 32

#define AIN2 0
#define AIN1 2
#define BIN1 33
#define BIN2 32

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, AIN1, AIN2, BIN1, BIN2);
// Stepper stepper(STEPS, AIN1, AIN2, BIN2, BIN1);
// Stepper stepper(STEPS, AIN2, AIN1, BIN1, BIN2);
// Stepper stepper(STEPS, AIN2, AIN1, BIN2, BIN1);

// Stepper stepper(STEPS, AIN1, BIN1, AIN2, BIN2);
//Stepper stepper(STEPS, AIN2, BIN1, AIN1, BIN2);
// Stepper stepper(STEPS, AIN2, BIN2, AIN1, BIN1);



void setup()
{
  Serial.begin(115200);
  Serial.println("Stepper test!");

  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(5);
}

void loop()
{
  Serial.println("Forward");
  stepper.step(10);

  delay(3000);

  Serial.println("Backward");
  stepper.step(-10);

  delay(3000);
}