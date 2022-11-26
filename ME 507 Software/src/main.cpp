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
#include "Stepper.h"
#include <WiFi.h>
#include <WebServer.h>
#include "imu.h"

//-------------------------------------------------------










//-----------------------WiFi----------------------------

// Make esp32 create its own access point
#undef USE_LAN

const char* ssid = "trackPlat";   // SSID, network name seen on LAN lists
const char* password = "kevin123";   // ESP32 WiFi password (min. 8 characters)

IPAddress local_ip (192, 168, 5, 1); // Address of ESP32 on its own network
IPAddress gateway (192, 168, 5, 1);  // The ESP32 acts as its own gateway
IPAddress subnet (255, 255, 255, 0); // Network mask; just leave this as is

WebServer server (80);

//-------------------------------------------------------










//--------------------Define Pins------------------------

// GPS Pins
#define RX2 GPIO_NUM_16
#define TX2 GPIO_NUM_17

// Motor 1 Pins
#define AIN2 0
#define AIN1 2
#define BIN1 33
#define BIN2 32

#define motX2 0
#define motX1 2
#define motX3 33
#define motX4 32

#define motY2 25
#define motY1 26
#define motY3 12
#define motY4 14

//-------------------------------------------------------










//--------------------Define Constants------------------------

// GPS Pins
#define RX2 GPIO_NUM_16
#define TX2 GPIO_NUM_17

// Motor Constants
#define STEPS 200
#define MOTOR_SPEED 50
#define DEG_PER_STEP 1.8

//-------------------------------------------------------










//-----------------Instantiate Objects-------------------

// IMU Objects
Adafruit_LSM6DSOX lsm6ds;
Adafruit_LIS3MDL lis3mdl;
Imu iamyou;
sensors_event_t accel, gyro, mag, temp;
sensors_vec_t angles;

// GPS Objects
Adafruit_GPS GPS(&Serial2);

// Stepper Motor Objects
// Stepper stepperX(STEPS, AIN1, BIN1, AIN2, BIN2);
Stepper stepperX(STEPS, motX1, motX2, motX3, motX4); // For Breakout board
Stepper stepperY(STEPS, motY1, motY2, motY3, motY4); // For Breakout board

//-------------------------------------------------------










//------------------Task Periods [ms]---------------------

#define IMU_DELAY 20
#define MOTOR_DELAY 50
#define GPS_DELAY 1000
#define POSITION_DELAY 1000
#define SEND_DELAY 500

//-------------------------------------------------------










//-------------------Shares & Queues----------------------

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
Share<float> thetaX("Theta X");
Share<float> thetaY("Theta Y");
Share<float> heading("Heading");

//-------------------------------------------------------










//----------------------Functions-------------------------

void HTML_header(String& a_string, const char* page_title)
{
    a_string += "<!DOCTYPE html> <html>\n";
    a_string += "<head><meta name=\"viewport\" content=\"width=device-width,";
    a_string += " initial-scale=1.0, user-scalable=no\">\n<title> ";
    a_string += page_title;
    a_string += "</title>\n";
    a_string += "<style>html { font-family: Helvetica; display: inline-block;";
    a_string += " margin: 0px auto; text-align: center;}\n";
    a_string += "body{margin-top: 50px;} h1 {color: #4444AA;margin: 50px auto 30px;}\n";
    a_string += "p {font-size: 24px;color: #222222;margin-bottom: 10px;}\n";
    a_string += "</style>\n</head>\n";
}

void handle_DocumentRoot()
{
    // Serial.println("Request from client");
    String a_str;
    HTML_header (a_str, "Main Page");
    a_str += "<body>\n<div id=\"webpage\">\n";
    a_str += "<h1>TrackPlat</h1>\n";

    a_str += "<p>";
    a_str += "Satellites in view: ";
    a_str += siv.get();
    a_str += "\n</p>";

    a_str += "<p>";
    a_str += "Latitude: ";
    a_str += latitude.get();
    a_str += "\n</p>";

    a_str += "<p>";
    a_str += "Longitude: ";
    a_str += longitude.get();
    a_str += "\n</p>";

    a_str += "</div>\n</body>\n</html>\n";

    server.send (200, "text/html", a_str); 
}

void handle_NotFound(void)
{
    server.send (404, "text/plain", "Not found");
}

//-------------------------------------------------------










//----------------------Tasks----------------------------

void task_imu(void* p_params)
{
  while (true)
  {
    // Serial.println("Task IMU");

    // Get platform positons from IMU, put them in queues
    // sensors_event_t accel, gyro, mag, temp;

    /* Get new normalized sensor events */
    

    /* Get new normalized sensor events */
    lsm6ds.getEvent(&accel, &gyro, &temp);
    lis3mdl.getEvent(&mag);
    angles = iamyou.get_angles(accel.acceleration);
    //Serial.printf("Accelerations: (%f, %f, %f) m/s^2\n", accel.acceleration.x, accel.acceleration.y, accel.acceleration.z);
    //Serial.printf("Angles: (%f, %f, %f) deg\n\n", angles.x, angles.y, angles.z);

    thetaX.put(angles.x);
    thetaY.put(angles.y);
    heading.put(1); //add heading from mag

    // Wait
    vTaskDelay(IMU_DELAY);
  }
}

void task_motor(void* p_params)
{
  float stepX;
  float stepY;

  while (true)
  {
    // Serial.println("Task Motor");

    // If target is at a higher theta value, step positive
    // int8_t valX = targetX.get() - thetaX.get();
    // if (valX>0) stepX = 1;
    // else if (valX<0) stepX = -1;
    // else stepX = 0;

    // int8_t valY = targetY.get() - thetaY.get();
    // if (valY>0) stepY = 1;
    // else if (valY<0) stepY = -1;
    // else stepY = 0;

    float e_x = thetaX.get()-targetX.get();
    float e_y = thetaY.get()-targetY.get();
    float dead_band =5; //deg
    if (e_x >dead_band)
    {stepY = -1;}
    else if (e_x < -dead_band)
    {stepY = +1;}
    else
    {stepY = 0;}
    if (e_y >dead_band)
    {stepX = +1;}
    else if (e_y < -dead_band)
    {stepX = -1;}
    else
    {stepX = 0;}

    stepperX.step(stepX);
    stepperY.step(stepY);

    // Wait
    vTaskDelay(MOTOR_DELAY);
  }
}

void task_gps(void* p_params)
{
  while (true)
  {
    // Serial.println("Task GPS");

    // Check GPS
    // GPS.read();

    // Try to parse data
    if (!GPS.parse(GPS.lastNMEA())) {}

    // If the data was parsed correctly, update shares
    else
    {
      latitude.put(GPS.latitude);
      longitude.put(GPS.longitude);
      altitude.put(GPS.altitude);
      siv.put(GPS.satellites);
    }

    // Wait
    vTaskDelay(GPS_DELAY);
  }
}

void task_position(void* p_params)
{
  while (true)
  {
    // Serial.println("Task Position");
    // Get data
    // Get: thetaX, thetaY, heading

    // Run calculations

    // Update targets
    targetX.put(0);
    targetY.put(0);

    // Wait
    vTaskDelay(POSITION_DELAY);
  }
}

void task_send(void* p_params)
{
  server.on("/", handle_DocumentRoot);
  server.onNotFound(handle_NotFound);

  server.begin ();
  Serial.println ("HTTP server started");

  while (true)
  {
    // Send data
    server.handleClient();

    // Wait
    vTaskDelay(SEND_DELAY);
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

  // Begin wifi
  WiFi.mode (WIFI_AP);
  WiFi.softAPConfig (local_ip, gateway, subnet);
  WiFi.softAP (ssid, password);

  Serial.println("Starting");

  // Setup tasks
  xTaskCreate(task_imu, "IMU Task", 4096, NULL, 10, NULL);
  xTaskCreate(task_motor, "Motor Task", 1024, NULL, 8, NULL);
  xTaskCreate(task_gps, "GPS Task", 1024, NULL, 6, NULL);
  xTaskCreate(task_send, "Send Task", 8192, NULL, 4, NULL);
  xTaskCreate(task_position, "Position Calculation Task", 1024, NULL, 2, NULL);

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

  // Setup for Motors
  stepperX.setSpeed(MOTOR_SPEED);
  stepperY.setSpeed(MOTOR_SPEED);
}

void loop()
{
  // Cannot be run inside an interrupt
  GPS.read();
  GPS.parse(GPS.lastNMEA());

}

//-------------------------------------------------------