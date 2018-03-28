#ifndef GPS_MUTEX_H
#define GPS_MUTEX_H

#include <Arduino.h>

class GPSMutex
{
public:
    GPSMutex();
    virtual ~GPSMutex();

    bool acquire(int timeoutMs=100);
    bool release();

private:
    static SemaphoreHandle_t _gpsMutex;

};

#endif
