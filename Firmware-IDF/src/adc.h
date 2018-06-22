#ifndef _ADC_H_
#define _ADC_H_

#include "defines.h"
#include "ADS1015.h"

class ADC
{
    public:
    
    //Singleton
    static ADC* instance();


    float read_voltage();
    float read_current();
    

    private:

    static ADC* _instance;

    ADC();

    ADS1015 _ads1015;

};


#endif
