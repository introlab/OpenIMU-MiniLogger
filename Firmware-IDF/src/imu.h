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

    SemaphoreHandle_t getSemaphore() {return _semaphore;}

    protected:

    IMU();

   void setup_interrupt_pin();

    MPU9250 _mpu9250;

    TaskHandle_t _readIMUHandle;
    SemaphoreHandle_t _semaphore;

    static IMU * _instance;

};
#endif