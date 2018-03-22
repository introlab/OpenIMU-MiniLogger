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

union IMUDataPoint
{
    imuData_t data;
    uint8_t bytes[36];
};

class IMU
{
public:
    IMU();
    virtual ~IMU();

    void begin();

    void startSerialLogging();
    void stopSerialLogging();

    void startQueueLogging(QueueHandle_t queue);
    void stopQueueLogging();
};

#endif
