#ifndef _POWER_H_
#define _POWER_H_

#include "defines.h"
#include "ADS1015.h"

class Power
{
    friend void powerTask(void *pvParameters);

    public:
    
    //Singleton
    static Power* instance();

    void enableExternalPower();
    void disableExternalPower();
    float last_voltage(); 
    float last_current();
    float read_voltage();
    float read_current();

    private:

    static Power* _instance;

    Power();
    void lock();
    void unlock();

    ADS1015 _ads1015;

    TaskHandle_t _powerTaskHandle;
    SemaphoreHandle_t _mutex;

    float _last_voltage;
    float _last_current;

    protected:


};


#endif
