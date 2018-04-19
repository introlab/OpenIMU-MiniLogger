#ifndef IMU_H
#define IMU_H

#include <SparkFunMPU9250-DMP.h>
#include <Arduino.h>

typedef struct {
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    float magX;
    float magY;
    float magZ;
} imuData_t;

typedef imuData_t* imuData_ptr;

class IMU
{
public:
    IMU();
    virtual ~IMU();

    void begin();

    void startSerialLogging();
    void stopSerialLogging();

    void startQueueLogging(QueueHandle_t queue, SemaphoreHandle_t semaphore);
    void stopQueueLogging();
};

#endif
