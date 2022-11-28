/** @file imu.h
 *  header file for Imu Class
 *  @author    Kevin Lee
 *  @date      2022-Nov-25
 */
#ifndef IMU_H // include guard
#define IMU_H

#include "Arduino.h"
#include "cmath"
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LIS3MDL.h>

/** @brief   Imu Class
 *  @details Contains methods dealing with IMU data parsing
 */
class Imu
{
     private :
         sensors_vec_t accel;
         float m;
         sensors_vec_t ang;
     public :
          Imu(void);
          sensors_vec_t get_angles(sensors_vec_t a);
          sensors_vec_t get_error(sensors_vec_t a, sensors_vec_t a_c);
          sensors_vec_t get_motor_ctrl(sensors_vec_t a, sensors_vec_t a_c, float dead_band);
};
 
#endif