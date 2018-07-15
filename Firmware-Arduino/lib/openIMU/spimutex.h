#ifndef SPI_MUTEX_H
#define SPI_MUTEX_H

#include <Arduino.h>

class SPIMutex
{
public:
    SPIMutex();
    virtual ~SPIMutex();

    bool acquire(int timeoutMs = 100);
    bool release();

private:
    static SemaphoreHandle_t _mutex;

};

#endif
