#include "display.h"
#include "fonts.h"
namespace
{
  SPIMutex _mutex;


}

Display::Display()
{

}

Display::~Display()
{

}

void Display::begin()
{
  if(_mutex.acquire(100))
  {
    _epd.Init();
    _mutex.release();
  }
  else
    Serial.println("Unable to acquire SPI mutex.");

}

void Display::end()
{
  if(_mutex.acquire(100))
  {
    _epd.Sleep();
    _mutex.release();
  }
  else
    Serial.println("Unable to acquire SPI mutex.");

}

void Display::showSplashScreen()
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);

    _redPaint.DrawStringAt(10, 20, "Open IMU", &Font24, 0);
    _blackPaint.DrawStringAt(10, 50, "System is idle", &Font16, 0);

    _blackPaint.DrawStringAt(10, 245, "SW2 to start...", &Font16, 0);

    if(_mutex.acquire(100))
    {
      _epd.DisplayFrame(_blackImage, _redImage);
      _mutex.release();
    }
    else
      Serial.println("Unable to acquire SPI mutex.");
}

void Display::showMenu(Menu* menu)
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);

    menu->paint(_blackPaint, _redPaint, 5, 5);

    if(_mutex.acquire(100))
    {
      _epd.DisplayFrame(_blackImage, _redImage);
      _mutex.release();
    }
    else
      Serial.println("Unable to acquire SPI mutex.");
}

void Display::updateMenu(Menu* menu)
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);

    menu->paint(_blackPaint, _redPaint, 5, 5);
    if(_mutex.acquire(100))
    {
      _epd.DisplayFrame(_blackImage, _redImage);
      _mutex.release();
    }
    else
      Serial.println("Unable to acquire SPI mutex.");
}
