/**
 * @file main.cpp
 * @author Kevin and the Gang
 * @brief 
 * @version 0.1
 * @date 2022-11-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//-----------------------Libraries------------------------

#include <Arduino.h>
#include "taskshare.h"
#include "taskqueue.h"

//-------------------------------------------------------










//------------------Task Periods [ms]---------------------

#define IMU_DELAY 2
#define MOTOR_DELAY 5
#define GPS_DELAY 1000
#define CALCULATION_DELAY 1000
#define SEND_DELAY 2

//-------------------------------------------------------










//-------------------Shares & Queues----------------------

// Send Data
Share<bool> newData("New Data Flag");

// GPS Data
Queue<int16_t> satHeadings(100, "Satellite Headings");
Share<float> latitude("Latitude");
Share<float> longitude("Longitude");
Share<float> altitude("Altitude");
Share<uint8_t> siv("Satellites in View");

// Target Data
Share<float> targetX("Target Theta X");
Share<float> targetY("Target Theta Y");

// IMU Data
Queue<float> thetaX(10, "Theta X");
Queue<float> thetaY(10, "Theta Y");
Queue<float> heading(10, "Heading");

//-------------------------------------------------------










//----------------------Tasks----------------------------

void task_imu(void* p_params)
{
  while (true)
  {
    // Get platform positons from IMU, put them in queues
    thetaX.put(NULL);
    thetaY.put(NULL);
    heading.put(NULL);

    // Wait
    vTaskDelay(IMU_DELAY);
  }
}

void task_motor(void* p_params)
{
  while (true)
  {
    // Run motor controllers
    // Get: thetaX, thetaY, heading, targetX, targetY

    // Wait
    vTaskDelay(MOTOR_DELAY);
  }
}

void task_gps(void* p_params)
{
  while (true)
  {
    Serial.println("GPS Task");

    // Get gps data
    latitude.put(NULL);
    longitude.put(NULL);
    altitude.put(NULL);
    siv.put(NULL);

    // Set new data flag
    newData.put(true);

    // Wait
    vTaskDelay(GPS_DELAY);
  }
}

void task_send(void* p_params)
{
  while (true)
  {
    if (newData.get())
    {
      // Reset flag
      newData.put(false);
      
      // Get data
      // Get: latitude, longitude, altitude, SIV

      // Send data

    }

    // Wait
    vTaskDelay(SEND_DELAY);
  }
}

void task_position(void* p_params)
{
  while (true)
  {
    // Get data
    // Get: thetaX, thetaY, heading

    // Run calculations

    // Update targets
    targetX.put(0);
    targetY.put(0);

    // Wait
    vTaskDelay(CALCULATION_DELAY);
  }
}

//-------------------------------------------------------










//---------------------Program---------------------------

void setup()
{
  // Begin serial channel
  Serial.begin(115200);
  if (!Serial) {}
  Serial.println("Starting");

  // Setup tasks
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