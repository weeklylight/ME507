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
#include "Adafruit_LSM6DSOX.h"
#include "Adafruit_LIS3MDL.h"
#include "Adafruit_GPS.h"

//-------------------------------------------------------










//--------------------Define Pins------------------------

// GPS Pins
#define RX2 GPIO_NUM_16
#define TX2 GPIO_NUM_17

//-------------------------------------------------------










//-----------------Instantiate Objects-------------------

// IMU Objects
Adafruit_LSM6DSOX lsm6ds;
Adafruit_LIS3MDL lis3mdl;

// GPS Objects
Adafruit_GPS GPS(&Serial2);

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
    sensors_event_t accel, gyro, mag, temp;

   /* Get new normalized sensor events */
   lsm6ds.getEvent(&accel, &gyro, &temp);
   lis3mdl.getEvent(&mag);

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

    // Check GPS
    GPS.read();

    // Try to parse data
    if (!GPS.parse(GPS.lastNMEA()))
    {
      // If the data can't be parsed, do nothing
    }

    // If the data was parsed correctly, update shares
    else
    {
      latitude.put(GPS.latitude);
      longitude.put(GPS.longitude);
      altitude.put(GPS.altitude);
      siv.put(GPS.satellites);

      // Get satHeadings ?

      // Set new data flag
      newData.put(true);
    }

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
      Serial.printf("Latitude: %f\n", latitude.get());
      Serial.printf("Longitude: %f\n", longitude.get());
      Serial.printf("Altitude: %f\n", altitude.get());
      Serial.printf("Satellites in View: %d\n", siv.get());

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
  // Begin serial channels
  Serial.begin(115200);
  if (!Serial) {}

  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); // GPS serial channel
  if (!Serial2) {}

  Serial.println("Starting");

  // Setup tasks
  xTaskCreate(task_imu, "IMU Position", 1024, NULL, 10, NULL);
  xTaskCreate(task_motor, "Motor Controller", 1024, NULL, 8, NULL);
  xTaskCreate(task_gps, "GPS Data", 1024, NULL, 6, NULL);
  xTaskCreate(task_send, "Send Data", 1024, NULL, 4, NULL);
  xTaskCreate(task_position, "Position Calculation", 1024, NULL, 2, NULL);

  Serial.println("Tasks created");

  // Setup for IMU
  bool lsm6ds_success = lsm6ds.begin_I2C(0x6A);
  bool lis3mdl_success = lis3mdl.begin_I2C(0x1E);
  lis3mdl.setDataRate(LIS3MDL_DATARATE_155_HZ); // Set data rate
  lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS); // Set range
  lis3mdl.setPerformanceMode(LIS3MDL_MEDIUMMODE); // Set performance
  lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE); // Set mode
  lis3mdl.setIntThreshold(500);
  lis3mdl.configInterrupt(false, false,
                          true, // Enable z axis
                          true, // Polarity
                          false, // Don't latch
                          true); // Enabled!

  Serial.println("IMU done");
  
  // Setup for GPS
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); // Set NMEA sentence type
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ); // Set output rate

  Serial.println("GPS done");
}

void loop()
{
  // Do nothing
}

//-------------------------------------------------------