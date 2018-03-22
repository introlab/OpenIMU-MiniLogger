#ifndef IMU_H
#define IMU_H

#include <SparkFunMPU9250-DMP.h>

class IMU
{
public:
    IMU();
    virtual ~IMU();

    void begin();

    void startSerialLogging();
    void stopSerialLogging();
};

#endif
