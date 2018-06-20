#ifndef _IMU_H_
#define _IMU_H_

#if 0
#include "MPU9250.h"

#define INTERRUPT_PIN 34
#define MPU9250_I2C_ADDRESS 0x68

class IMU
{
    public:
    IMU();

    protected:

    MPU9250 _imu;
};

#endif
#endif