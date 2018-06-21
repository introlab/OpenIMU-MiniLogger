#ifndef _IMU_H_
#define _IMU_H_


#include "MPU9250.h"

#define INTERRUPT_PIN 34
#define MPU9250_I2C_ADDRESS 0x68

class IMU
{
    public:
   
    static IMU* instance();
    
    void readSensor();

    protected:

    IMU();

   

    MPU9250 _mpu9250;

    static IMU * _instance;

};
#endif