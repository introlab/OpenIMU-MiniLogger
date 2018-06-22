#include "adc.h"
#include "defines.h"

ADC* ADC::_instance = NULL;

ADC* ADC::instance()
{
    if (ADC::_instance == NULL)
        ADC::_instance = new ADC();
    return ADC::_instance;
}

ADC::ADC()
    : _ads1015(I2C_NUM_1)
{
    
}


float ADC::read_voltage()
{
    uint16_t value = _ads1015.readADC_SingleEnded(ADC_VOLTAGE_CHANNEL);
    //printf("VOLTAGE HEX: %4.4x, %i\n", value, value);
    return 5.0 * 0.002 * (float) value;
}

float ADC::read_current()
{
    uint16_t value = _ads1015.readADC_SingleEnded(ADC_CURRENT_CHANNEL);
    //printf("CURRENT HEX: %4.4x, %i\n", value, value);
    return ((0.002 * (float) value) - (3.1/2.0)) / 5.0;
}