/** @file imu.cpp
 *  This file prints arrays with various arguments
 *  @author    Kevin Lee
 *  @date      2022-Oct-25
 */
#include "imu.h"
#include "cmath"

/** @brief   StatTracker constructor
 *  @details initializes count, sum and sum_sq
 */
Imu :: Imu(void) 
{
    //initialize vars
    sensors_vec_t accel;
    m = 0;
    sensors_vec_t ang;
}

/** @brief   adds data to StatTracker
 *  @details overloaded method that accepts a float
 *  @param data input float data
 */
sensors_vec_t Imu::get_angles(sensors_vec_t a)
{
    accel.x = a.x;
    accel.y = a.y;
    accel.z = a.z;
    m = sqrt(accel.x*accel.x+accel.y*accel.y+accel.z*accel.z);
    ang.x = +asin(accel.x/m)*180/3.142;
    ang.y = +asin(accel.y/m)*180/3.142;
    return ang;
}
sensors_vec_t Imu::get_error(sensors_vec_t a, sensors_vec_t a_c)
{
    sensors_vec_t out;
    out.x = a.x-a_c.x;
    out.y = a.y -a_c.y;
    out.z = a.z - a_c.z;
    return out;
}
sensors_vec_t Imu::get_motor_ctrl(sensors_vec_t a, sensors_vec_t a_c, float dead_band)
{
    sensors_vec_t e = get_error(a,a_c);
    sensors_vec_t out;
    if (e.x >dead_band)
    {out.y = -1;}
    else if (e.x < -dead_band)
    {out.y = +1;}
    else
    {out.y = 0;}
    if (e.y >dead_band)
    {out.x = +1;}
    else if (e.y < -dead_band)
    {out.x = -1;}
    else
    {out.x = 0;}
    out.z = 0;
    return out;
}