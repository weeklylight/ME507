#include <Arduino.h>

#define l1 15


void setup()
{
  Serial.begin(115200);
  if (!Serial) {}
  Serial.println("starting");
  
  pinMode(l1, INPUT);
}

void loop()
{
  if (digitalRead(l1))
  {
    Serial.println("Contact made!");
  }
}