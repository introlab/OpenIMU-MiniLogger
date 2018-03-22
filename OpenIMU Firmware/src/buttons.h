#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include <Adafruit_MCP23008.h>

class Buttons
{
public:
    Buttons();
    virtual ~Buttons();
    void begin();

    int getActionCtn();
    int getPreviousCtn();
    int getNextCtn();

    void decrementActionCtn();
    void decrementPreviousCtn();
    void decrementNextCtn();
};

#endif
