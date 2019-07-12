/*
 * Widget to display battery level on Open IMU homesreen
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

#include "widget/battery.h"
#include "ssd1331.h"
#include <sstream>

#define X_ORIGIN 0
#define Y_ORIGIN 12

#define ICON_WIDTH 10
#define ICON_HEIGHT 25

#define BATT_LEVEL_LOW 3.3
#define BATT_LEVEL_1 3.5
#define BATT_LEVEL_2 3.7
#define BATT_LEVEL_3 3.9
#define BATT_LEVEL_4 4.1

using namespace Widget;

const unsigned char icon[] = {
  0x3f, 0x00, 
  0x21, 0x00, 
  0xff, 0xc0, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0x80, 0x40, 
  0xff, 0xc0
};

/**
 * Construct a battery widget
 */
Battery::Battery() : AbstractWidget(X_ORIGIN, Y_ORIGIN, nullptr) { }

/**
 * Update the battery widget displayed value
 * 
 * @param float voltage: current battery voltage in volt
 * @param float current: current battery current in ampere
 */
void Battery::updateValue(float voltage, float current)
{
    _voltage = voltage;
    _current = current;
    paintLogo();

    if(_selected)
    {
        paintMessage();
    }
}

/**
 * Pain the battery logo to the screen
 * Logo adjust according to current battery voltage
 */
void Battery::paintLogo()
{
    if (_voltage > BATT_LEVEL_LOW)
    {
        SSD1331_line(_xorigin + 1, _yorigin + 1, _xorigin + WIDGET_WIDTH - 2, _yorigin + WIDGET_HEIGHT - 2, BLACK);
        SSD1331_line(_xorigin + 1, _yorigin + WIDGET_HEIGHT - 2, _xorigin + WIDGET_WIDTH - 2, _yorigin + 1, BLACK);
        SSD1331_mono_bitmap(_xorigin + 7, _yorigin + 4, icon, ICON_WIDTH, ICON_HEIGHT, WHITE);
    }

    if (_voltage > BATT_LEVEL_1)
    {
        SSD1331_rectangle(_xorigin + 9, _yorigin + 23, _xorigin + 14, _yorigin + 26, WHITE, true);
    }
    else
    {
        SSD1331_rectangle(_xorigin + 9, _yorigin + 23, _xorigin + 14, _yorigin + 26, BLACK, true);
    }

    if (_voltage > BATT_LEVEL_2)
    {
        SSD1331_rectangle(_xorigin + 9, _yorigin + 18, _xorigin + 14, _yorigin + 21, WHITE, true);
    }
    else
    {
        SSD1331_rectangle(_xorigin + 9, _yorigin + 18, _xorigin + 14, _yorigin + 21, BLACK, true);
    }

    if (_voltage > BATT_LEVEL_3)
    {
        SSD1331_rectangle(_xorigin + 9, _yorigin + 13, _xorigin + 14, _yorigin + 16, WHITE, true);
    }
    else
    {
        SSD1331_rectangle(_xorigin + 9, _yorigin + 13, _xorigin + 14, _yorigin + 16, BLACK, true);
    }

    if (_voltage > BATT_LEVEL_4)
    {
        SSD1331_rectangle(_xorigin + 9, _yorigin + 8, _xorigin + 14, _yorigin + 11, WHITE, true);
    }
    else
    {
        SSD1331_rectangle(_xorigin + 9, _yorigin + 8, _xorigin + 14, _yorigin + 11, BLACK, true);
    } 

    if (_voltage <= BATT_LEVEL_LOW)
    {
        SSD1331_mono_bitmap(_xorigin + 7, _yorigin + 4, icon, ICON_WIDTH, ICON_HEIGHT, RED);
        SSD1331_line(_xorigin + 1, _yorigin + 1, _xorigin + WIDGET_WIDTH - 2, _yorigin + WIDGET_HEIGHT - 2, RED);
        SSD1331_line(_xorigin + 1, _yorigin + WIDGET_HEIGHT - 2, _xorigin + WIDGET_WIDTH - 2, _yorigin + 1, RED);
    }
}

/**
 * Get the message to be displayed when the battery widget is selected
 * 
 * @return string message: current battery metric formatted xx V  xx A
 */
std::string Battery::getMessage()
{
    std::stringstream msgStream;
    msgStream << _voltage << " V  " << _current << " A";

    return msgStream.str();
}