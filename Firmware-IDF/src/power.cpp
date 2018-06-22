#include "power.h"
#include "defines.h"

Power* Power::_instance = NULL;

Power* Power::instance()
{
    if (Power::_instance == NULL)
        Power::_instance = new Power();
    return Power::_instance;
}

Power::Power()
    : _ads1015(I2C_NUM_1)
{
    
}


float Power::read_voltage()
{
    uint16_t value = _ads1015.readADC_SingleEnded(ADC_VOLTAGE_CHANNEL);
    //printf("VOLTAGE HEX: %4.4x, %i\n", value, value);
    return 5.0 * 0.002 * (float) value;
}

float Power::read_current()
{
    uint16_t value = _ads1015.readADC_SingleEnded(ADC_CURRENT_CHANNEL);
    //printf("CURRENT HEX: %4.4x, %i\n", value, value);
    return ((0.002 * (float) value) - (3.1/2.0)) / 5.0;
}