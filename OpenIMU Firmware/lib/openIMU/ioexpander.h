#ifndef IO_EXPANDER_H
#define IO_EXPANDER_H

#include <Arduino.h>
#include <Adafruit_MCP23008.h>
#include "i2cmutex.h"

class IOExpander
{
public:
    IOExpander();
    virtual ~IOExpander();

    void begin();

    void pinMode(uint8_t pinNo, uint8_t mode);
    void pullUp(uint8_t pinNo, uint8_t mode);

    void digitalWrite(uint8_t pinNo, uint8_t value);
    uint8_t digitalRead(uint8_t pinNo);

private:
    static Adafruit_MCP23008 _mcp;
    static I2CMutex _i2c;
    static bool hasBegun;
};

#endif
