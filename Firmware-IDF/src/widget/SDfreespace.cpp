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
#include "widget/SDfreespace.h"
#include "ssd1331.h"

#define X_ORIGIN 73
#define Y_ORIGIN 16

#define ICON_WIDTH 16
#define ICON_HEIGHT 22


using namespace Widget;



const unsigned char icon[] = {0x1F, 0xFF, 0x24, 0x95, 0x24, 0x95, 0x24, 0x95, 
0x24, 0x95, 0x24, 0x95, 0x24, 0x95, 0x20, 0x01, 
0x20, 0x01, 0x40, 0x01, 0x81, 0x81, 0x8D, 0xB1, 
0xCF, 0xF1, 0x47, 0xE1, 0x5E, 0x79, 0xDE, 0x79, 
0x87, 0xE1, 0x8F, 0xF1, 0x8D, 0xB1, 0x81, 0x81, 
0x80, 0x01, 0xFF, 0xFF};

/**
 * Construct a SampleRate widget
 */
SDFreeSpace::SDFreeSpace():AbstractWidget(X_ORIGIN, Y_ORIGIN, nullptr){ }

/**
 * Paint the SampleRate widget icon to the screen
 * Icon shows current fix status
 */
void SDFreeSpace::paintLogo()
{
        SSD1331_mono_bitmap(_xorigin + 5, _yorigin + 4, icon, ICON_WIDTH, ICON_HEIGHT, WHITE);
    
}
void SDFreeSpace::setStatus(float freeSpace)
{
    _freeSpace=freeSpace;
}
/**
 * Return the message that should be displayed when the widget is selected
 */
std::string SDFreeSpace::getMessage()
{

    std::stringstream msgStream;

    msgStream<<"Free:"<< _freeSpace << "GB" ;
    return msgStream.str();
    
}