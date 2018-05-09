#include "ioexpander.h"

SPIMutex IOExpander::_mutex;
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
        if(_mutex.acquire(100))
        {
          _mcp.begin();
          _mutex.release();
          hasBegun = true;
        }
        else
          Serial.println("Error acquire mutex, IOExpander::begin()");
    }
}

void IOExpander::pinMode(uint8_t pinNo, uint8_t mode)
{
    if (_mutex.acquire(100))
    {
      _mcp.pinMode(pinNo, mode);
      _mutex.release();
    }
    else
      Serial.println("Error acquire mutex, IOExpander::pinMode()");
}

void IOExpander::pullUp(uint8_t pinNo, uint8_t mode)
{
    if (_mutex.acquire(100))
    {
      _mcp.pullupMode(pinNo, mode);
      _mutex.release();
    }
    else
      Serial.println("Error acquire mutex, IOExpander::pullUp()");
}

void IOExpander::pullupMode(uint8_t pinNo, uint8_t mode)
{
    pullUp(pinNo, mode);
}


void IOExpander::digitalWrite(uint8_t pinNo, uint8_t value)
{
    if (_mutex.acquire(100))
    {
      _mcp.digitalWrite(pinNo, value);
      _mutex.release();
    }
    else
      Serial.println("Error acquire mutex, IOExpander::digitalWrite()");
}

uint8_t IOExpander::digitalRead(uint8_t pinNo)
{
    uint8_t value = 0;
    if (_mutex.acquire(100))
    {
      value = _mcp.digitalRead(pinNo);
      _mutex.release();
    }
    else
        Serial.println("Error acquire mutex, IOExpander::digitalRead()");

    return value;
}
