#ifndef _ADC_H_
#define _ADC_H_

#include <stdio.h>
#include "driver/i2c.h"


/*!

    Inspired by:

    @file     Adafruit_ADS1015.cpp
    @author   K.Townsend (Adafruit Industries)
    @license  BSD (see license.txt)

    Driver for the ADS1015/ADS1115 ADC

    This is a library for the Adafruit MPL115A2 breakout
    ----> https://www.adafruit.com/products/???

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0 - First release
*/

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
    #define ADS1015_ADDRESS                 (0x48)    // 1001 000 (ADDR = GND)
/*=========================================================================*/


class ADC
{
    

    public:
    ADC(i2c_port_t port, uint8_t address=ADS1015_ADDRESS)
        : _port(port), _address(address)
    {
        setup();
    }

    void setup();


    float read_voltage();
    float read_current();
    uint16_t readADC_SingleEnded(uint8_t channel);

    SemaphoreHandle_t getSemaphore() {return _semaphore;}

    private:

    

    uint16_t readRegister(uint8_t i2cAddress, uint8_t reg);
    esp_err_t writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value);

    i2c_port_t _port;
    uint8_t _address;
    SemaphoreHandle_t _semaphore;

    

};


#endif
