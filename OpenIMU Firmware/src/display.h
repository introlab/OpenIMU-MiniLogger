#ifndef DISPLAY_H
#define DISPLAY_H

#include "epd2in7b.h"
#include "epdpaint.h"

#include "menu.h"

class Display
{
public:
    Display();
    ~Display();

    void begin();
    void end();

    void showSplashScreen();
    void showMenu(Menu* menu);
    void updateMenu(Menu* menu);

private:
    unsigned char _blackImage[EPD_WIDTH*EPD_HEIGHT];
    unsigned char _redImage[EPD_WIDTH*EPD_HEIGHT];

    Epd _epd;
    Paint _blackPaint{_blackImage, EPD_WIDTH, EPD_HEIGHT};
    Paint _redPaint{_redImage, EPD_WIDTH, EPD_HEIGHT};
};

#endif /* end of include guard:  */
