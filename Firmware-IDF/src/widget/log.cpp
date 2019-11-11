/*
 * Widget to toggle data logging on Open IMU homescreen
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

#include "widget/log.h"
#include "ssd1331.h"

#define X_ORIGIN 48
#define Y_ORIGIN 16

#define ICON_WIDTH 18
#define ICON_HEIGHT 21

using namespace Widget;

const unsigned char icon[] = {
  0x00, 0x70, 0x00, 
  0x00, 0x50, 0x00, 
  0x00, 0x50, 0x00, 
  0x00, 0x50, 0x00, 
  0x01, 0xdc, 0x00, 
  0x01, 0x04, 0x00, 
  0x00, 0x88, 0x00, 
  0x00, 0x88, 0x00, 
  0x00, 0x50, 0x00, 
  0x1f, 0x77, 0xc0, 
  0x10, 0x20, 0x40, 
  0xff, 0xfe, 0x40, 
  0x80, 0x01, 0x40, 
  0x80, 0x01, 0x40, 
  0x40, 0x01, 0x40, 
  0x40, 0x01, 0xc0, 
  0x40, 0x00, 0xc0, 
  0x20, 0x00, 0xc0, 
  0x20, 0x00, 0xc0, 
  0x20, 0x00, 0x40, 
  0x3f, 0xff, 0xc0
};



/**
 * Construct a log widget
 * 
 * @param void (*)() toggleLog : function to toggle logging on and off
 */
Log::Log(void (*toggleLog)()) : AbstractWidget(X_ORIGIN, Y_ORIGIN, toggleLog) { }

void Log::setStatus(bool isLogging,bool isSDCardPresent)
{
    _isLogging = isLogging;
    _isSDCardPresent = isSDCardPresent;

    if (_visible) paint(true);
}

/**
 * Paint the log widget icon to the screen
 * Icon shows current log status
 */
void Log::paintLogo()
{
    if (_isLogging && _isSDCardPresent)
    {
        SSD1331_mono_bitmap(_xorigin + 2, _yorigin + 6, icon, ICON_WIDTH, ICON_HEIGHT, BLUE);
    }
    else if (!_isLogging && _isSDCardPresent)
    {
        SSD1331_line(_xorigin + 1, _yorigin + 1, _xorigin + WIDGET_WIDTH - 2, _yorigin + WIDGET_HEIGHT - 2, WHITE);
        SSD1331_line(_xorigin + 1, _yorigin + WIDGET_HEIGHT - 2, _xorigin + WIDGET_WIDTH - 2, _yorigin + 1, WHITE);
        SSD1331_mono_bitmap(_xorigin + 2, _yorigin + 6, icon, ICON_WIDTH, ICON_HEIGHT, WHITE);
    }
    else if (!_isSDCardPresent)
    {
        SSD1331_line(_xorigin + 1, _yorigin + 1, _xorigin + WIDGET_WIDTH - 2, _yorigin + WIDGET_HEIGHT - 2, RED);
        SSD1331_line(_xorigin + 1, _yorigin + WIDGET_HEIGHT - 2, _xorigin + WIDGET_WIDTH - 2, _yorigin + 1, RED);
        SSD1331_mono_bitmap(_xorigin + 2, _yorigin + 6, icon, ICON_WIDTH, ICON_HEIGHT, RED);
    }
    
}

/**
 * Return the message that should be displayed when the widget is selected
 * 
 * @return string message: a string indicating the action performed on click
 */
std::string Log::getMessage()
{
    if (_isLogging && _isSDCardPresent)
    {
        return "Stop Log";  
    }
    else if(!_isLogging && _isSDCardPresent)
    {
        return "Start Log";
    }
    else
    {
        return "No SD Card";
    }
    
}