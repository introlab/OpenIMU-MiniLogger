#ifndef _POWER_H_
#define _POWER_H_

#include "defines.h"
#include "ADS1015.h"

class Power
{
    public:
    
    //Singleton
    static Power* instance();


    float read_voltage();
    float read_current();
    
    void enableExternalPower();
    void disableExternalPower();

    private:

    static Power* _instance;

    Power();

    ADS1015 _ads1015;

    TaskHandle_t _powerTaskHandle;

};


#endif
