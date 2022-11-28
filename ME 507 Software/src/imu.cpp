/** @file imu.cpp
 *  contains methods for Imu class for data parsing
 *  @author    Kevin Lee
 *  @date      2022-Nov-25
 */
#include "imu.h"
#include "cmath"

/** @brief   Imu constructor
 *  @details initializesaccel, m, ang
 */
Imu :: Imu(void) 
{
    //initialize vars
    sensors_vec_t accel;
    m = 0;
    sensors_vec_t ang;
}

/** @brief   gets angles from accelerations
 *  @details converts acceleration data to x y and z angles in local coordinates
 *  @param a sensors_vec_t data for acceleration
 *  @returns sensors_vec_t [x_angle, y_angle, z_angle]
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
/** @brief   gets error between anles
 *  @details gets error vector from the input vector and control vector
 *  @param a sensors_vec_t actual angle [x,y,z]
 *  @param a_c sensors_vec_t control angle [x,y,z]
 *  @returns sensors_vec_t error vector representing difference between actual and control
 */
sensors_vec_t Imu::get_error(sensors_vec_t a, sensors_vec_t a_c)
{
    sensors_vec_t out;
    out.x = a.x-a_c.x;
    out.y = a.y -a_c.y;
    out.z = a.z - a_c.z;
    return out;
}
/** @brief   calculates motor control
 *  @details greturns a 1 0 or -1 depending on what direction motors need to turn
 *  @param a sensors_vec_t actual angle [x,y,z]
 *  @param a_c sensors_vec_t control angle [x,y,z]
 *  @param dead_band angle in degrees considered acceptable where no rotation is needed
 *  @returns sensors_vec_t motor control [x motor, y motor, 0] 1 is clockwise, -1 is counter clockwise
 */
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