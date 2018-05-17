#include "display.h"
#include "fonts.h"
#include <string>
#include <sstream>

Display::Display()
{

}

Display::~Display()
{

}

void Display::begin()
{
  _epd.Init();
}

void Display::end()
{
  _epd.Sleep();
}

void Display::showSplashScreen()
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);

    _redPaint.DrawStringAt(10, 20, "Open IMU", &Font24, 0);
    _blackPaint.DrawStringAt(10, 50, "System is idle", &Font16, 0);
    _blackPaint.DrawStringAt(10, 245, "SW2 to start...", &Font16, 0);

    _epd.DisplayFrame(_blackImage, _redImage);

}

void Display::showMenu(Menu* menu)
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);
    //menu->paint(_blackPaint, _redPaint, 5, 5);
    menu->paint(_blackPaint, _blackPaint, 5, 5);
    _epd.DisplayFrame(_blackImage, NULL /*_redImage */);

}

void Display::updateMenu(Menu* menu)
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);
    //menu->paint(_blackPaint, _redPaint, 5, 5);
    menu->paint(_blackPaint, _blackPaint, 5, 5);
    _epd.DisplayFrame(_blackImage, NULL /*_redImage*/);
}

void Display::clear()
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);
    _epd.DisplayFrame(_blackImage, _redImage);
}

void Display::displayVoltage(float volts, float current)
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);

    std::stringstream batt_text;

    batt_text << "B: " << volts << "V " << current << "A";

    _blackPaint.DrawStringAt(0, 0, batt_text.str().c_str(), &Font16, 0);

    _epd.DisplayFrame(_blackImage, NULL/*_redImage */);
}
