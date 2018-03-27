#ifndef I2C_MUTEX_H
#define I2C_MUTEX_H

#include <Arduino.h>

class I2CMutex
{
public:
    I2CMutex();
    virtual ~I2CMutex();

    bool acquire(int timeoutMs);
    bool release();

private:
    static SemaphoreHandle_t _i2cMutex;

};

#endif
