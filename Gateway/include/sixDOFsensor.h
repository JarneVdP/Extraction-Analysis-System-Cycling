#ifndef SIXDOFSENSOR_H
#define SIXDOFSENSOR_H

#include "I2Cdev.h"
#include "MPU6050.h"

extern MPU6050 accelgyro;

extern int16_t ax, ay, az;
extern int16_t gx, gy, gz;

void sixDOFsensor();
#endif