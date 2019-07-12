/*
 * Widget to display current GPS status on Open IMU homesreen
 * author: Cedric Godin
 * 
 * Copyright 2019 IntRoLab
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "widget/gps.h"
#include "ssd1331.h"

#define X_ORIGIN 24
#define Y_ORIGIN 12

#define ICON_WIDTH 13
#define ICON_HEIGHT 26


using namespace Widget;

const unsigned char icon[] = {
  0x0f, 0x80, 
  0x30, 0x60, 
  0x60, 0x30, 
  0x4f, 0x90, 
  0x90, 0x48, 
  0x90, 0x48, 
  0x90, 0x48, 
  0x90, 0x48, 
  0x90, 0x48, 
  0x4d, 0x90, 
  0x42, 0x10, 
  0x40, 0x10, 
  0x20, 0x10, 
  0x20, 0x20, 
  0x30, 0x20, 
  0x10, 0x40, 
  0x10, 0x40, 
  0x08, 0x80, 
  0x0c, 0x80, 
  0x05, 0x80, 
  0x37, 0x60, 
  0x43, 0x10, 
  0x82, 0x08, 
  0x80, 0x08, 
  0x60, 0x30, 
  0x1f, 0xc0
};

/**
 * Construct a GPS widget
 */
GPS::GPS() : AbstractWidget(X_ORIGIN, Y_ORIGIN, nullptr) { }

void GPS::setStatus(bool hasFix)
{
    _hasFix = hasFix;

    paintLogo();
    if (_selected)
    {
        paintMessage();
    }
}

/**
 * Paint the GPS widget icon to the screen
 * Icon shows current fix status
 */
void GPS::paintLogo()
{
    if (_hasFix)
    {
        SSD1331_line(_xorigin + 1, _yorigin + 1, _xorigin + WIDGET_WIDTH - 2, _yorigin + WIDGET_HEIGHT - 2, BLACK);
        SSD1331_line(_xorigin + 1, _yorigin + WIDGET_HEIGHT - 2, _xorigin + WIDGET_WIDTH - 2, _yorigin + 1, BLACK);
        SSD1331_mono_bitmap(_xorigin + 5, _yorigin + 4, icon, ICON_WIDTH, ICON_HEIGHT, WHITE);
    }
    else
    {
        SSD1331_line(_xorigin + 1, _yorigin + 1, _xorigin + WIDGET_WIDTH - 2, _yorigin + WIDGET_HEIGHT - 2, RED);
        SSD1331_line(_xorigin + 1, _yorigin + WIDGET_HEIGHT - 2, _xorigin + WIDGET_WIDTH - 2, _yorigin + 1, RED);
        SSD1331_mono_bitmap(_xorigin + 5, _yorigin + 4, icon, ICON_WIDTH, ICON_HEIGHT, RED);
    }
    
}

/**
 * Return the message that should be displayed when the widget is selected
 * 
 * @return string message: a string indicating if the GPS has a fix
 */
std::string GPS::getMessage()
{
    if (_hasFix)
    {
        return "GPS fix";
    }
    else
    {
        return "No GPS fix";
    }
    
}