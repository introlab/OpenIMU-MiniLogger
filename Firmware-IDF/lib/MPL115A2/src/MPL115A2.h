#ifndef _MPL115A2_H_
#define _MPL115A2_H_

#include <stdio.h>
#include "i2cbus.h"


/**************************************************************************/
/*!
    Inspired from:

    @file     Adafruit_MPL115A2.h
    @author   K. Townsend (Adafruit Industries)
    @license  BSD (see license.txt)

    This is a library for the Adafruit MPL115A2 breakout board
    ----> https://www.adafruit.com/products/???

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0  - First release
*/
/**************************************************************************/

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
    #define MPL115A2_ADDRESS                       (0x60)    // 1100000
/*=========================================================================*/

/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/
    #define MPL115A2_REGISTER_PRESSURE_MSB         (0x00)
    #define MPL115A2_REGISTER_PRESSURE_LSB         (0x01)
    #define MPL115A2_REGISTER_TEMP_MSB             (0x02)
    #define MPL115A2_REGISTER_TEMP_LSB             (0x03)
    #define MPL115A2_REGISTER_A0_COEFF_MSB         (0x04)
    #define MPL115A2_REGISTER_A0_COEFF_LSB         (0x05)
    #define MPL115A2_REGISTER_B1_COEFF_MSB         (0x06)
    #define MPL115A2_REGISTER_B1_COEFF_LSB         (0x07)
    #define MPL115A2_REGISTER_B2_COEFF_MSB         (0x08)
    #define MPL115A2_REGISTER_B2_COEFF_LSB         (0x09)
    #define MPL115A2_REGISTER_C12_COEFF_MSB        (0x0A)
    #define MPL115A2_REGISTER_C12_COEFF_LSB        (0x0B)
    #define MPL115A2_REGISTER_STARTCONVERSION      (0x12)
/*=========================================================================*/

typedef struct {
    int16_t a0coeff;
    int16_t b1coeff;
    int16_t b2coeff;
    int16_t c12coeff;
} coefdata_t;



class MPL115A2
{
    public:
    MPL115A2(i2c_port_t port, uint8_t address=MPL115A2_ADDRESS);

    float getPressure(void);
    float getTemperature(void);
    void getPT(float &P, float &T);

    private:
    
    i2c_port_t _port;
    uint8_t _address;

    float _mpl115a2_a0;
    float _mpl115a2_b1;
    float _mpl115a2_b2;
    float _mpl115a2_c12;

    void readCoefficients(void);
    esp_err_t readRegisters(uint8_t reg, uint8_t count, uint8_t* dest);
    esp_err_t writeRegister(uint8_t reg, uint8_t data);

};

#endif
