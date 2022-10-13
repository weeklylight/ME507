/** /file main.cpp
 *  /brief main setup and loop for uC
 */

#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  while(!Serial) {}

  Serial.print("started");
  pinMode(2,OUTPUT);
}

void loop() {
  digitalWrite(2,LOW);
  vTaskDelay(1000);
  digitalWrite(2,HIGH);
  Serial.print("run ");
  vTaskDelay(1000);
}