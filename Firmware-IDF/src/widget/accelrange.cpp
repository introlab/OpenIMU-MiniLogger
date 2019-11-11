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
#include <sstream>
#include "widget/accelrange.h"
#include "ssd1331.h"

#define X_ORIGIN 48
#define Y_ORIGIN 16

#define ICON_WIDTH 16
#define ICON_HEIGHT 26


using namespace Widget;



const unsigned char icon[] = {0x00, 0x00, 0x02, 0x00, 0x07, 0x00, 0x0A, 0x80, 
0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x10, 
0x07, 0x08, 0x07, 0xFC, 0x0F, 0x08, 0x50, 0x10, 
0x60, 0x00, 0x70, 0x00, 0x00, 0x00, 0x03, 0x80, 
0x04, 0x40, 0x04, 0x40, 0x04, 0x40, 0x03, 0xC0, 
0x00, 0x40, 0x00, 0x40, 0x04, 0x40, 0x03, 0x80, 
0x00, 0x00, 0x00, 0x00};

/**
 * Construct a SampleRate widget
 */
AccelRange::AccelRange(void (*toggleChangeRange)()) : AbstractWidget(X_ORIGIN, Y_ORIGIN, toggleChangeRange) { }

/**
 * Paint the SampleRate widget icon to the screen
 * Icon shows current fix status
 */
void AccelRange::paintLogo()
{
        SSD1331_mono_bitmap(_xorigin + 5, _yorigin + 4, icon, ICON_WIDTH, ICON_HEIGHT, WHITE);
    
}
void AccelRange::setStatus(int AccelRange)
{
    if(AccelRange==1) _rangeG=2;
    if(AccelRange==2) _rangeG=4;
    if(AccelRange==3) _rangeG=8;
    if(AccelRange==4) _rangeG=16;
}
/**
 * Return the message that should be displayed when the widget is selected
 */
std::string AccelRange::getMessage()
{

    std::stringstream msgStream;

    msgStream<<"Accel Rg:"<< _rangeG << "g" ;
    return msgStream.str();
    
}