#include <Arduino.h>
#include <Stepper.h>

// https://learn.adafruit.com/adafruit-tb6612-h-bridge-dc-stepper-motor-driver-breakout/using-stepper-motors

// change this to the number of steps on your motor
#define STEPS 200
#define WAIT 200

// Motor 1 Pins
#define motX1 2 // AIN1
#define motX2 0 // AIN2
#define motX3 33 // BIN1
#define motX4 32 // BIN2

// Motor 2 Pins
#define motY1 26 // BIN1
#define motY2 25 // BIN2
#define motY3 12 // AIN1
#define motY4 14 // AIN2


Stepper stepperX(STEPS, motX1, motX2, motX3, motX4); // For Breakout board
Stepper stepperY(STEPS, motY1, motY2, motY3, motY4); // For Breakout board




void setup()
{
  Serial.begin(115200);
  Serial.println("Stepper test!");

  // set the speed of the motor to 30 RPMs
  stepperX.setSpeed(50);
}

void loop()
{
  Serial.println("Forward");
  for (int i = 0; i < 20; i++)
  {
    stepperX.step(1);
    delay(WAIT);
  }

  delay(1000);

  Serial.println("Backward");
  // for (int i = 0; i < 20; i++)
  // {
  //   stepperX.step(-1);
  //   delay(WAIT);
  // }
  stepperX.step(-20);

  delay(1000);

}