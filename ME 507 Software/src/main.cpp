/**
 * @file main.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>
#include "taskshare.h"
#include "taskqueue.h"

// Define period for tasks in mS
#define IMU_DELAY 2
#define MOTOR_DELAY 5
#define GPS_DELAY 1000
#define CALCULATION_DELAY 1000
#define SEND_DELAY 2

// Define shares and queues
Share<bool> newData("New Data Flag");


//----------------------Tasks----------------------------
void task_imu(void* p_params)
{
  // Run continuosly
  while (true)
  {
    // Do something

    // Wait
    vTaskDelay(IMU_DELAY);
  }
}

void task_motor(void* p_params)
{
  // Run continuosly
  while (true)
  {
    // Do something

    // Wait
    vTaskDelay(MOTOR_DELAY);
  }
}

void task_gps(void* p_params)
{
  // Run continuosly
  while (true)
  {
    Serial.println("GPS Task");

    // Set new data flag
    newData.put(true);

    // Get gps data

    // Wait
    vTaskDelay(GPS_DELAY);
  }
}

void task_send(void* p_params)
{
  // Run continuosly
  while (true)
  {
    if (newData.get())
    {
      // Reset flag
      newData.put(false);
      Serial.println("Sending data");

      // Send data
    }

    // Wait
    vTaskDelay(SEND_DELAY);
  }
}

void task_position(void* p_params)
{
  // Run continuosly
  while (true)
  {
    // Run calculations

    // Wait
    vTaskDelay(CALCULATION_DELAY);
  }
}

//-------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  if (!Serial) {}

  Serial.println("Starting");

  xTaskCreate(task_imu, "IMU Position", 1024, NULL, 10, NULL);
  xTaskCreate(task_motor, "Motor Controller", 1024, NULL, 8, NULL);
  xTaskCreate(task_gps, "GPS Data", 1024, NULL, 6, NULL);
  xTaskCreate(task_send, "Send Data", 1024, NULL, 4, NULL);
  xTaskCreate(task_position, "Position Calculation", 1024, NULL, 2, NULL);
  
  Serial.println("Tasks Created");
}

void loop()
{
  // Do nothing
}
//-------------------------------------------------------