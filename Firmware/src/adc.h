#ifndef __ADC_H__
#define __ADC_H__

#include <Arduino.h>
#include <Adafruit_ADS1015.h>


class ADC
{
public:
    ADC();
    virtual ~ADC();

    void begin();

    void startSerialLogging();
    void stopSerialLogging();

    void startQueueLogging(QueueHandle_t queue, SemaphoreHandle_t semaphore);
    void stopQueueLogging();
  protected:

};


#endif //__ADC_H__
