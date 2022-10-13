/** /file main.cpp
 *  /brief main setup and loop for uC
 */

#include <Arduino.h>
#include <Adafruit_LSM6DSOX.h>

// Adafruit_LSM6DSOX sox;

void task_1(void* p_params)
{
  Serial.print("task_1");
  while(true)
  {
    Serial.print("a");
    vTaskDelay(2000);
  }
}

void task_2(void* p_params)
{
  Serial.print("task_2");
  while(true)
  {
    Serial.print("b");
    vTaskDelay(5000);
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial) {}

  xTaskCreate(task_1, "task_1", 2048, NULL, 3, NULL);
  xTaskCreate(task_2, "task_2", 2048, NULL, 5, NULL);

  Serial.print("started");
  pinMode(2,OUTPUT);
}

void loop() {
  // digitalWrite(2,LOW);
  // vTaskDelay(1000);
  // digitalWrite(2,HIGH);
  // Serial.print("run ");
  // vTaskDelay(1000);

  // sensors_event_t accel;
  // sensors_event_t gyro;
  // sensors_event_t temp;
  // sox.getEvent(&accel, &gyro, &temp);
}