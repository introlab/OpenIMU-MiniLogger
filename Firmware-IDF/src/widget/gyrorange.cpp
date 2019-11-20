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
#include "widget/gyrorange.h"
#include "ssd1331.h"

#define X_ORIGIN 24
#define Y_ORIGIN 16

#define ICON_WIDTH 16
#define ICON_HEIGHT 26


using namespace Widget;



const unsigned char icon[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x02, 0x07, 0xC6, 0x18, 0x3C, 0x23, 0x88, 
0x46, 0xC4, 0x44, 0x44, 0x9F, 0xF2, 0xB4, 0x5A, 
0xA4, 0x4A, 0xB4, 0x5A, 0x9F, 0xF2, 0x44, 0x44, 
0x46, 0xC4, 0x23, 0x88, 0x78, 0x30, 0xC7, 0xC0, 
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00};

/**
 * Construct a SampleRate widget
 */
GyroRange::GyroRange(void (*toggleChangeRange)()) : AbstractWidget(X_ORIGIN, Y_ORIGIN, toggleChangeRange) { }

/**
 * Paint the SampleRate widget icon to the screen
 * Icon shows current fix status
 */
void GyroRange::paintLogo()
{
        SSD1331_mono_bitmap(_xorigin + 5, _yorigin + 4, icon, ICON_WIDTH, ICON_HEIGHT, WHITE);
    
}
void GyroRange::setStatus(int GyroRange)
{
    if(GyroRange==1) _rangeDPS=250;
    if(GyroRange==2) _rangeDPS=500;
    if(GyroRange==3) _rangeDPS=1000;
    if(GyroRange==4) _rangeDPS=2000;
}
/**
 * Return the message that should be displayed when the widget is selected
 */
std::string GyroRange::getMessage()
{

    std::stringstream msgStream;

    msgStream<<"Gyro Rg:"<< _rangeDPS << "dps" ;
    return msgStream.str();
    
}