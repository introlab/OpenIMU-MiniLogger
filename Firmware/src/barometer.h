#ifndef __BAROMETER_H_
#define __BAROMETER_H_

#include <Arduino.h>
#include "Adafruit_MPL115A2.h"

typedef struct {
    float temperature;
    float pressure;
} baroData_t;

typedef baroData_t* baroData_ptr;


class Barometer
{
public:
    Barometer();
    virtual ~Barometer();

    void begin();

    void startQueueLogging(QueueHandle_t queue, SemaphoreHandle_t semaphore);
    void stopQueueLogging();
};

#endif
