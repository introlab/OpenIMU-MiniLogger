#ifndef _IOEXPANDER_H_
#define _IOEXPANDER_H_

#include "defines.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

/**
 
  This adapted from:
  MCP23S17.h  Version 0.2
  Microchip MCP23S17 SPI I/O Expander Class for Arduino
  Created by Cort Buffington & Keith Neufeld
  March, 2011
  January, 2013
  January, 2015
  February, 2016
 **/




class IOExpander
{
    public:

    IOExpander(int addr, gpio_num_t cs_pin, spi_host_device_t host_device);

    esp_err_t pullupMode(uint8_t pin, uint8_t mode);

    esp_err_t pinMode(uint8_t pin, uint8_t mode);

    uint8_t digitalRead(uint8_t pin);

    esp_err_t digitalWrite(uint8_t pin, uint8_t value);

    protected:

    void setup();
   
    esp_err_t byteWrite(uint8_t reg, uint8_t value);

    esp_err_t wordWrite(uint8_t reg, unsigned int word);

    int _address;
    int _cs_pin;
    spi_host_device_t _host_device;
    spi_device_handle_t _handle;
    spi_device_interface_config_t _config;

    //Cached states
    unsigned int _modeCache;// Caches the mode (input/output) configuration of I/O pins
    unsigned int _pullupCache;// Caches the internal pull-up configuration of input pins (values persist across mode changes)
    unsigned int _invertCache;// Caches the input pin inversion selection (values persist across mode changes)
    unsigned int _outputCache;// Caches the output pin state of pins

};






#endif