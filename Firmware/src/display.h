#ifndef DISPLAY_H
#define DISPLAY_H

#include "epd1in54b.h"
#include "epdpaint.h"
#include "menu.h"
#include "spimutex.h"

class Display
{
public:
    Display();
    ~Display();

    void begin();
    void end();

    void showSplashScreen();
    void clear();
    void showMenu(Menu* menu);
    void updateMenu(Menu* menu, bool stateLog);
    void displayVoltage(float volts, float current,bool validData, bool stateLog, bool sdLog);

private:
    unsigned char _blackImage[EPD_WIDTH*EPD_HEIGHT];
    unsigned char _redImage[EPD_WIDTH*EPD_HEIGHT];

    Epd _epd;
    Paint _blackPaint{_blackImage, EPD_WIDTH, EPD_HEIGHT};
    Paint _redPaint{_redImage, EPD_WIDTH, EPD_HEIGHT};
};

#endif /* end of include guard:  */
