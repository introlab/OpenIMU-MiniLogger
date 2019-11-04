/*
 * Widget to toggle SD card connection on Open IMU homescreen
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

#include "widget/sd.h"
#include "ssd1331.h"

#define X_ORIGIN 73
#define Y_ORIGIN 16

#define SD_ICON_WIDTH 18
#define SD_ICON_HEIGHT 24

#define USB_ICON_WIDTH 15
#define USB_ICON_HEIGHT 25

using namespace Widget;

const unsigned char sd_icon[] {
  0x0f, 0xff, 0xc0, 
  0x10, 0x00, 0x40, 
  0x10, 0x00, 0x40, 
  0x20, 0x00, 0x40, 
  0x40, 0x00, 0x40, 
  0xc0, 0x00, 0x40, 
  0x80, 0x00, 0x40, 
  0x80, 0x00, 0x40, 
  0x80, 0x00, 0x40, 
  0x80, 0x00, 0x40, 
  0x80, 0x00, 0x40, 
  0x80, 0x00, 0x40, 
  0x9e, 0x78, 0x40, 
  0xb1, 0x46, 0x40, 
  0xa0, 0x43, 0x40, 
  0xa0, 0x41, 0x40, 
  0x9e, 0x41, 0x40, 
  0x83, 0x41, 0x40, 
  0x81, 0x41, 0x40, 
  0x81, 0x42, 0x40, 
  0xbe, 0x7c, 0x40, 
  0x80, 0x00, 0x40, 
  0x80, 0x00, 0x40, 
  0xff, 0xff, 0xc0
};

const unsigned char usb_icon[] {
  0x01, 0x00, 
  0x01, 0x00, 
  0x03, 0x80, 
  0x03, 0x80, 
  0x07, 0xc0, 
  0x01, 0x00, 
  0x01, 0x00, 
  0x01, 0x1e, 
  0x01, 0x1e, 
  0x61, 0x1e, 
  0xf1, 0x1e, 
  0xf1, 0x04, 
  0x61, 0x0c, 
  0x41, 0x18, 
  0x61, 0xf0, 
  0x31, 0x00, 
  0x1d, 0x00, 
  0x03, 0x00, 
  0x01, 0x00, 
  0x01, 0x00, 
  0x03, 0x80, 
  0x07, 0xc0, 
  0x07, 0xc0, 
  0x07, 0xc0, 
  0x03, 0x80
};

/**
 * Construct a SD card widget
 * 
 * @param void (*)() toggleExternal : function to toggle SD card between ESP32 and USB storage
 */
SD::SD(void (*toggleExternal)()) : AbstractWidget(X_ORIGIN, Y_ORIGIN, toggleExternal) { }

void SD::setStatus(bool isExternal)
{
    _isExternal = isExternal;
    if (_visible) paint(true);
}

/**
 * Paint the SD card widget icon to the screen
 * Icon shows current SD connection
 */
void SD::paintLogo()
{
    if (_isExternal)
    {
        SSD1331_mono_bitmap(_xorigin + 4, _yorigin + 4, usb_icon, USB_ICON_WIDTH, USB_ICON_HEIGHT, WHITE);
    }
    else
    {
        SSD1331_mono_bitmap(_xorigin + 2, _yorigin + 4, sd_icon, SD_ICON_WIDTH, SD_ICON_HEIGHT, BLUE);
    }
    
}

/**
 * Return the message that should be displayed when the widget is selected
 * 
 * @return string message: a string indicating the action performed on click
 */
std::string SD::getMessage()
{
    if (_isExternal)
    {
        return "Disconnect USB";
    }
    else
    {
        return "Connect USB";
    }
    
}