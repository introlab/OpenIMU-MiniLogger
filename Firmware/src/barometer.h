#ifndef __BAROMETER_H_
#define __BAROMETER_H_

#include <Arduino.h>
#include "Adafruit_MPL115A2.h"

class Barometer
{
public:
    Barometer();
    virtual ~Barometer();

    void begin();

    void startSerialLogging();
    void stopSerialLogging();

    void startQueueLogging(QueueHandle_t queue, SemaphoreHandle_t semaphore);
    void stopQueueLogging();
};

#endif
