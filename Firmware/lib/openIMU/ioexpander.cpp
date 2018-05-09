#include "ioexpander.h"

I2CMutex IOExpander::_i2c;
MCP IOExpander::_mcp(0,5); //ADDR 0, CS=PIN5
bool IOExpander::hasBegun = false;

IOExpander::IOExpander()
{

}

IOExpander::~IOExpander()
{

}

void IOExpander::begin()
{
    if(!hasBegun) {
        _i2c.acquire(100);
        _mcp.begin();
        _i2c.release();
        hasBegun = true;
    }
}

void IOExpander::pinMode(uint8_t pinNo, uint8_t mode)
{
    _i2c.acquire(100);
    _mcp.pinMode(pinNo, mode);
    _i2c.release();
}

void IOExpander::pullUp(uint8_t pinNo, uint8_t mode)
{
    _i2c.acquire(100);
    _mcp.pullupMode(pinNo, mode);
    _i2c.release();
}

void IOExpander::pullupMode(uint8_t pinNo, uint8_t mode)
{
    pullUp(pinNo, mode);
}


void IOExpander::digitalWrite(uint8_t pinNo, uint8_t value)
{
    _i2c.acquire(100);
    _mcp.digitalWrite(pinNo, value);
    _i2c.release();
}

uint8_t IOExpander::digitalRead(uint8_t pinNo)
{
    uint8_t value;
    _i2c.acquire(100);
    value = _mcp.digitalRead(pinNo);
    _i2c.release();

    return value;
}
