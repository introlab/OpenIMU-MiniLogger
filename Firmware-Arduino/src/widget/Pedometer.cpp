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
#include "widget/Pedometer.h"
#include "ssd1331.h"

#define X_ORIGIN 24
#define Y_ORIGIN 16

#define ICON_WIDTH 15
#define ICON_HEIGHT 25


using namespace Widget;


const unsigned char icon[] = {0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x1C, 0x00, 
0x3E, 0x00, 0x3E, 0x00, 0x3E, 0x00, 0x3E, 0x30, 
0x3E, 0x38, 0x3E, 0x3C, 0x3E, 0x7C, 0x1C, 0x7C, 
0x00, 0x7C, 0x3C, 0x7C, 0x3C, 0x7C, 0x1C, 0x7C, 
0x00, 0x38, 0x00, 0x38, 0x00, 0x00, 0x00, 0x78, 
0x00, 0x78, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00};

/**
 * Construct a Pedometer widget
 */
Pedometer::Pedometer() : AbstractWidget(X_ORIGIN, Y_ORIGIN, nullptr) { }

/**
 * Paint the SampleRate widget icon to the screen
 * Icon shows current fix status
 */
void Pedometer::paintLogo()
{
        SSD1331_mono_bitmap(_xorigin + 5, _yorigin + 4, icon, ICON_WIDTH, ICON_HEIGHT, WHITE);
    
}
void Pedometer::update(int stepcount,int steptime)
{
    _StepCount=stepcount;
    _StepTime=steptime;
}
/**
 * Return the message that should be displayed when the widget is selected
 */
std::string Pedometer::getMessage()
{

    std::stringstream msgStream;

    msgStream<<"Steps:"<< _StepCount;
    return msgStream.str();
    
}