/**
 * @file main.cpp
 * @author Kevin Lee
 * @author Jakob Frabosilio
 * @author Alexander Dunn
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










//----------------------Settings-------------------------

// Uncomment this line to provide print statements.
#define DEBUG ///< A line to allow for debugging print statements during operation.

#define SIV_TRACKER true

// Uncomment this line to simply level the platform
// #define LEVEL ///< A line to enable or disable simple leveling

//-------------------------------------------------------










//------------------Task Periods [ms]---------------------

#define IMU_DELAY 40 ///< The task delay in milliseconds for the IMU task.
#define MOTOR_DELAY 100 ///< The task delay in milliseconds for the motor task.

#define GPS_DELAY 1000 ///< The task delay in milliseconds for the GPS task.
#define POSITION_DELAY 1000 ///< The task delay in milliseconds for the position calculation task.
#define SEND_DELAY 500 ///< The task delay in milliseconds for the IoT task.
#define SIV_DELAY 2500 ///< The task delay in milliseconds for the satellite tracker task.

//-------------------------------------------------------










//--------------------Define Constants------------------------

// Motor Constants
#define STEPS 200 ///< The number of discrete steps for the stpper motors used.
#define MOTOR_SPEED 50 ///< The motor speed in rpm.
#define DEAD_BAND 5 ///< An acceptable plate angle error in degrees.

//-------------------------------------------------------










//--------------------Define Pins------------------------

// GPS Pins
#define RX2 GPIO_NUM_16 ///< The RX2 pin number used for the GPS.
#define TX2 GPIO_NUM_17 ///< The TX2 pin number used for the GPS.

// Motor 1 Pins
#define motX1 2 ///< The AIN1 pin for the X motor.
#define motX2 0 ///< The AIN2 pin for the X motor.
#define motX3 33 ///< The BIN1 pin for the X motor.
#define motX4 32 ///< The BIN2 pin for the X motor.

// Motor 2 Pins
#define motY1 26 ///< The BIN1 pin for the Y motor.
#define motY2 25 ///< The BIN2 pin for the Y motor.
#define motY3 12 ///< The AIN1 pin for the Y motor.
#define motY4 14 ///< The AIN2 pin for the Y motor.

// Limit Switch Pins
#define L1 36 ///< A pin to read the state of the first limit switch.
#define L2 39 ///< A pin to read the state of the second limit switch.
#define L3 34 ///< A pin to read the state of the third limit switch.
#define L4 35 ///< A pin to read the state of the forth limit switch.

//-------------------------------------------------------










//-----------------------WiFi----------------------------

// Make esp32 create its own access point
#undef USE_LAN

const char* ssid = "trackPlat"; ///< SSID, network name seen on LAN lists.
const char* password = "kevin123"; ///< ESP32 WiFi password (min. 8 characters).

IPAddress local_ip(192, 168, 5, 1); ///< Address of ESP32 on its own network.
IPAddress gateway(192, 168, 5, 1);  ///< The ESP32 acts as its own gateway.
IPAddress subnet(255, 255, 255, 0); ///< Network mask.

WebServer server(80); ///< The web server object used for IoT functionality.

//-------------------------------------------------------












//-----------------Instantiate Objects-------------------

// IMU Objects
Adafruit_LSM6DSOX lsm6ds;
Adafruit_LIS3MDL lis3mdl;
Imu iamyou;
sensors_event_t accel, gyro, mag, temp;
sensors_vec_t angles;

// GPS Objects
Adafruit_GPS GPS(&Serial2); ///< A GPS object.

// Stepper Motor Objects
Stepper stepperX(STEPS, motX1, motX2, motX3, motX4); ///< The X motor object.
Stepper stepperY(STEPS, motY1, motY2, motY3, motY4); ///< The Y motor object.

//-------------------------------------------------------










//-------------------Shares & Queues----------------------

// GPS Data
Share<float> latitude("Latitude"); ///< A shared variable to store the current latitude.
Share<float> longitude("Longitude"); ///< A shared variable to store the current longitude.
Share<float> altitude("Altitude"); ///< A shared variable to store the current altitude.
Share<uint8_t> siv("Satellites in View"); ///< A shared variable to store the current number of satellites in view.
Share<float> dop("Precision"); ///< A shared variable to store the current GPS precision.

// Target Data
Share<float> targetX("Target Theta X"); ///< A shared variable to store the current X angle target.
Share<float> targetY("Target Theta Y"); ///< A shared variable to store the current Y angle target.

// IMU Data
Share<float> thetaX("Theta X"); ///< A shared variable to store the current X angle.
Share<float> thetaY("Theta Y"); ///< A shared variable to store the current Y angle.

//-------------------------------------------------------










//----------------------Functions-------------------------

/**
 * @brief A function used to setup a webpage hosted by the ESP32.
 * 
 * @param a_string A reference to a string which will contain HTML information/
 * @param page_title A pointer to the location in memory storing the webpage title.
 */
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

/**
 * @brief A function to handle requests from the client.
 * @details This function runs anytime someone refreshes the page and updates the webpage with current data.
 * 
 */
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
    a_str += latitude.get()/100.0;
    a_str += "\n</p>";

    a_str += "<p>";
    a_str += "Longitude: ";
    a_str += longitude.get()/(-100.0);
    a_str += "\n</p>";

    a_str += "<p>";
    a_str += "Altitude: ";
    a_str += altitude.get();
    a_str += "\n</p>";

    a_str += "<p>";
    a_str += "PDOP: ";
    a_str += dop.get();
    a_str += "\n</p>";

    a_str += "</div>\n</body>\n</html>\n";

    server.send (200, "text/html", a_str); 
}

/**
 * @brief A function to handle 404 errors.
 * 
 */
void handle_NotFound(void)
{
    server.send (404, "text/plain", "Not found");
}

//-------------------------------------------------------










//----------------------Tasks----------------------------

/**
 * @brief The task used to control the IMU.
 * 
 * @param p_params A pointer to optional and unused parameters.
 */
void task_imu(void* p_params)
{
  while (true)
  {
    // Serial.println("Task IMU");

    /* Get new normalized sensor events */
    lsm6ds.getEvent(&accel, &gyro, &temp);
    angles = iamyou.get_angles(accel.acceleration);

    #ifdef DEBUG
      Serial.printf("Accelerations: (%f, %f, %f) m/s^2\n", accel.acceleration.x, accel.acceleration.y, accel.acceleration.z);
      Serial.printf("Angles: (%f, %f, %f) deg\n\n", angles.x, angles.y, angles.z);
    #endif

    thetaX.put(angles.x);
    thetaY.put(angles.y);

    // Wait
    vTaskDelay(IMU_DELAY);
  }
}

/**
 * @brief The task used to control both stepper motors.
 * 
 * @param p_params A pointer to optional and unused parameters.
 */
void task_motor(void* p_params)
{
  int8_t stepX; ///< The number of steps to move the X motor.
  int8_t stepY; ///< The number of steps to move the Y motor.

  uint8_t L1read = 0; ///< A variable to store the state of the first limit switch.
  uint8_t L2read = 0; ///< A variable to store the state of the second limit switch.
  uint8_t L3read = 0; ///< A variable to store the state of the third limit switch.
  uint8_t L4read = 0; ///< A variable to store the state of the fourth limit switch.


  while (true)
  {
    // Read from limit switches
    L1read = digitalRead(L1);
    L2read = digitalRead(L2);
    L3read = digitalRead(L3);
    L4read = digitalRead(L4);

    #ifdef DEBUG
      Serial.println("Limit Switches: ");
      Serial.print(L1read);
      Serial.print(" ");
      Serial.print(L2read);
      Serial.print(" ");
      Serial.print(L3read);
      Serial.print(" ");
      Serial.println(L4read);
    #endif

    float e_x = thetaX.get()-targetX.get(); ///< The proportional error for the X angle.
    float e_y = thetaY.get()-targetY.get(); ///< The proportional error for the Y angle.

    #ifdef DEBUG
      Serial.print("Errors: ");
      Serial.print(e_x);
      Serial.print(", ");
      Serial.println(e_y);
    #endif

    if (e_x > DEAD_BAND)
    {stepY = -1;}
    else if (e_x < -DEAD_BAND)
    {stepY = +1;}
    else
    {stepY = 0;}

    if (e_y > DEAD_BAND)
    {stepX = -1;}
    else if (e_y < -DEAD_BAND)
    {stepX = +1;}
    else
    {stepX = 0;}

    #ifdef DEBUG
      Serial.print("Steps: ");
      Serial.print(stepX);
      Serial.print(", ");
      Serial.println(stepY);
    #endif


    if ((stepY > 0) && (!L3read)) stepperY.step(stepY); // y+ and not L2
    else if ((stepY < 0) && (!L1read)) stepperY.step(stepY); // y- and not L4

    if ((stepX > 0) && (!L4read)) stepperX.step(stepX); // x+ and not L3
    else if ((stepX < 0) && (!L2read)) stepperX.step(stepX); // x- and not L1

    // Wait
    vTaskDelay(MOTOR_DELAY);
  }
}

/**
 * @brief The task used to control the GPS readings.
 * 
 * @param p_params A pointer to optional and unused parameters.
 */
void task_gps(void* p_params)
{
  while (true)
  {
    // Try to parse the data and do nothing if you can't
    if (!GPS.parse(GPS.lastNMEA())) {}

    // If the data was parsed correctly, update shares
    else
    {
      latitude.put(GPS.latitude);
      longitude.put(GPS.longitude);
      altitude.put(GPS.altitude);
      siv.put(GPS.satellites);
      dop.put(GPS.PDOP);
    }

    // Wait
    vTaskDelay(GPS_DELAY);
  }
}

/**
 * @brief The task used to update the webpage.
 * 
 * @param p_params A pointer to optional and unused parameters.
 */
void task_send(void* p_params)
{
  server.on("/", handle_DocumentRoot);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  while (true)
  {
    // Send data
    server.handleClient();

    // Wait
    vTaskDelay(SEND_DELAY);
  }
}

#ifdef LEVEL
  /**
   * @brief The task used to update the target platform position.
   * 
   * @param p_params A pointer to optional and unused parameters.
   */
  void task_position(void* p_params)
  {
    while (true)
    {
      {
      // Update targets
      targetX.put(0);
      targetY.put(0);

      // Wait
      vTaskDelay(POSITION_DELAY);
      }
    }
  }

#else
  /**
   * @brief The task used to find the optimal angle for getting a GPS signal.
   * 
   * @param p_params A pointer to optional and unused parameters.
   */
  void task_optimize_siv(void* p_params) 
  {
    // first, generate a grid of points to cycle through based on system geometry and desired # of points
    const uint8_t angleRes = 8; // resolution of skygrid in degrees
    const uint8_t maxAngle = 18; // maximum angle of device (based on geometry of device)
    const uint8_t divCount = maxAngle / angleRes; // number of cells in one direction, rounds down
    const uint8_t sivArraySize = (2*divCount + 1)*(2*divCount + 1); // generates an array for all points in grid

    int sivArrayX [sivArraySize] = {0}; // creates an array with (2n+1)^2 cells (a divCount of 2 makes 9 squares)
    int sivArrayY [sivArraySize] = {0}; // same as above; these arrays hold X,Y coordinates for each point of skygrid
    sivArrayX[0] = angleRes * divCount; // put initial value so upcoming for() loop can cycle through
    sivArrayY[0] = angleRes * divCount; // same as above; start at most positive location 
    int8_t dirFlag = -1; // flag that flips from positive to negative depending on direction
    bool flipFlag = true; // flag that tells if you just flipped or not

    // this algorithym generates the path that the platform will follow. it starts at the most positive X,Y location,
    // then snakes along the current row until it hits its negative-most X point. then, it goes to the next column,
    // and repeats this pattern (switching direction) until it hits the most negative X,Y location.
    // note that this is just generating a path using the first two values in this task and isn't actually actuating yet
    for (uint8_t i = 1; i < sivArraySize; i++)
    {
      if ((flipFlag == false) && (abs(sivArrayX[i-1]) == abs (angleRes * divCount)))
      {
        dirFlag = -1 * dirFlag;
        sivArrayY[i] = sivArrayY[i-1] - angleRes;
        sivArrayX[i] = sivArrayX[i-1];
        flipFlag = true;
      }
      else
      {
        sivArrayY[i] = sivArrayY[i-1];
        sivArrayX[i] = sivArrayX[i-1] + dirFlag * angleRes;
        flipFlag = false;
      }

    }

    uint8_t sivCount = 0; // counter variable to cycle through points on grid
    uint8_t sivArrayN [sivArraySize] = {0}; // array that stores how many satellites are spotted at each point in grid
    float sivArrayP [sivArraySize] = {0}; // array that stores PDOP value (measurement of precision for GPS) at each point
    float sivPMin = 10; // sets an arbitrary minimum value at 10 (PDOP values are generally <5, with smaller being better data)
    uint8_t sivMaxIndex = 0; // index variable that stores where minimum PDOP value was found
    Serial.println("Created array positions");
    vTaskDelay(5000); // wait 5s, for general debugging purposes

    // this is the main task loop 
    while (true)
    {
      if (SIV_TRACKER)
      {
        // this is the data collection loop. in this loop, the TrackPlat goes to a point in the
        // grid, hold position ofr a second, then takes data on where it is, how many satellites
        // it sees, and what the PDOP value is for the location. once the platform reaches the
        // final value in the grid, it breaks the loop and goes to analyze the data
        while(true)
        {
          targetX.put(sivArrayX[sivCount]);
          targetY.put(sivArrayY[sivCount]);

          vTaskDelay(SIV_DELAY);
          sivArrayN[sivCount] = siv.get();
          sivArrayP[sivCount] = dop.get();

          Serial.print(sivCount);
          Serial.print(": ");
          Serial.print(sivArrayX[sivCount]);
          Serial.print(", ");
          Serial.print(sivArrayY[sivCount]);
          Serial.print("; ");
          Serial.print(sivArrayN[sivCount]);
          Serial.print(" satellites in view, ");
          Serial.printf("%0.2fm of precision.\n", sivArrayP[sivCount]);
          sivCount++; 
          if (sivCount >= sivArraySize)
          {
            break;
          }
        }
        sivCount = 0;
        float sivPMinInit = sivArrayP[sivCount]; // notes the initial value of the PDOP array

        // this loop analyzes the PDOP data at each point. if the current PDOP value is less than
        // the value at the previous point, it has reached a new local minimum and overwrites it.
        // once the minimum PDOP value has been found (after going through all datapoints), it
        // exits the loop
        while(true)
        {
          if (sivArrayP[sivCount] <= sivPMin) 
          {
            sivPMin = sivArrayP[sivCount];
            sivMaxIndex = sivCount;
          }
          sivCount++;
          if (sivCount >= sivArraySize)
          {
            break;
          }
        }

        // if the initial PDOP value (at maximum positive X,Y) is the same as the minimum value
        // and the index of the minimum value is the final position, then most likely all of the
        // data for PDOP is the same (or similar enough to not worry). in this case, center the 
        // platform at 0,0 since position doesn't matter
        if ((sivPMinInit == sivPMin) && (sivMaxIndex == sivCount-1))
        {
          targetX.put(0);
          targetY.put(0);
          Serial.println();
          Serial.print("All positions similar; set to horiziontal position: (0, 0),");
        }

        // otherwise, set the position of the platform to whatever point in the grid contained the
        // minimum PDOP value
        else
        {
          targetX.put(sivArrayX[sivMaxIndex]);
          targetY.put(sivArrayY[sivMaxIndex]);
          Serial.println();
          Serial.print("Optimal position found: (");
        }
        Serial.print(sivArrayX[sivMaxIndex]);
        Serial.print(", ");
        Serial.print(sivArrayY[sivMaxIndex]);
        Serial.print("), ");
        Serial.print(sivArrayN[sivMaxIndex]);
        Serial.print(" satellites in view, ");
        Serial.printf("%0.2fm of precision.\n\n", sivArrayP[sivMaxIndex]);
        sivCount = 0;
        sivMaxIndex = 0;
        sivPMin = 10;
        
        // hold that position for a certain amount of time (can be configured elsewhere), then do
        // the searching algorythym and data collection again (since satellites move)
        vTaskDelay(SIV_DELAY*10);
      }
    }
  }
#endif

//-------------------------------------------------------










//---------------------Program---------------------------

/**
 * @brief The initial setup runs once to define pin modes, create tasks, and setup the peripherals.
 * 
 */
void setup()
{
  // Setup limit switch pins
  pinMode(L1, INPUT_PULLUP);
  pinMode(L2, INPUT_PULLUP);
  pinMode(L3, INPUT_PULLUP);
  pinMode(L4, INPUT_PULLUP);

  // Begin serial channels
  Serial.begin(115200);
  if (!Serial) {}

  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); // GPS serial channel
  if (!Serial2) {}

  // Begin wifi
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);

  Serial.println("Starting");

  // Setup tasks
  xTaskCreate(task_imu, "IMU Task", 4096, NULL, 10, NULL);
  xTaskCreate(task_motor, "Motor Task", 4096, NULL, 8, NULL);
  xTaskCreate(task_gps, "GPS Task", 1024, NULL, 6, NULL);
  xTaskCreate(task_send, "Send Task", 8192, NULL, 4, NULL);

  #ifdef LEVEL
    xTaskCreate(task_position, "Position Calculation Task", 1024, NULL, 2, NULL);
  #else
    xTaskCreate(task_optimize_siv, "Optimize Satellite Pos Task", 4096*2, NULL, 2, NULL);
  #endif

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
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGAGSA); // Set NMEA sentence type
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ); // Set output rate
  siv.put(0);
  dop.put(0);

  Serial.println("GPS done");

  // Setup for Motors
  stepperX.setSpeed(MOTOR_SPEED);
  stepperY.setSpeed(MOTOR_SPEED);

}

/**
 * @brief The loop runs continuously to handle interrupt based libraries such as the GPS.
 * 
 */
void loop()
{
  // Cannot be run inside an interrupt
  GPS.read();
  GPS.parse(GPS.lastNMEA());

}

//-------------------------------------------------------