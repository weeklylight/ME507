#include <Arduino.h>
#include <Stepper.h>

// change this to the number of steps on your motor
#define STEPS 200

#define AIN2 12
#define AIN1 14
#define BIN1 27
#define BIN2 26

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to

Stepper stepper(STEPS, AIN2, AIN1, BIN1, BIN2);


void setup()
{
  Serial.begin(115200);
  Serial.println("Stepper test!");

  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(60);
}

void loop()
{
  Serial.println("Forward");
  stepper.step(STEPS);
  Serial.println("Backward");
  stepper.step(-STEPS);
}