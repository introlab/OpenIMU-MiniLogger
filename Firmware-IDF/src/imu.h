#ifndef _IMU_H_
#define _IMU_H_

#include "defines.h"
//#include "MPU9250.h"
#include "SparkFunMPU9250-DMP.h"

#define INTERRUPT_PIN 34
#define MPU9250_I2C_ADDRESS 0x68

                                        

class IMU
{
    public:

    static IMU* instance();
    
    void readSensor(imuDataPtr_t data = NULL);

    SemaphoreHandle_t getSemaphore() {return _semaphore;}

    //Set the Frequency of the sample from the Module and only use the return when initialising the IMU for the sampleRate configuration.
    void setSampleRate(int rateHz);
    //Set all the parameter for the IMU
    void setIMUParameter(int rateHZ,int accelRange, int gyroRange);
    //Return 1,2,3,4 depending on the sample rate of the IMU
    int getSampleRate();
    //Return the number of step and the time form when it started taking data.
    int getStepCount();
    int getStepTime();

    protected:

    IMU();

    int SampleRateHz=10;
    int StepCount=0;
    int StepTime=0;

    void setup_interrupt_pin();

    MPU9250_DMP _mpu9250;

    TaskHandle_t _readIMUHandle;
    SemaphoreHandle_t _semaphore;

    static IMU * _instance;

};
#endif