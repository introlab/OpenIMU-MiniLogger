#ifndef IO_EXPANDER_H
#define IO_EXPANDER_H

#include <Arduino.h>
#include <MCP23S17.h>
#include "spimutex.h"

class IOExpander
{
public:
    IOExpander();
    virtual ~IOExpander();

    void begin();

    void pinMode(uint8_t pinNo, uint8_t mode);
    void pullUp(uint8_t pinNo, uint8_t mode);
    void pullupMode(uint8_t pinNo, uint8_t mode);

    void digitalWrite(uint8_t pinNo, uint8_t value);
    uint8_t digitalRead(uint8_t pinNo);

    bool acquire(time_t time=100);
    bool release();


private:
    static MCP _mcp;
    static SPIMutex _mutex;
    static bool hasBegun;
};

#endif
