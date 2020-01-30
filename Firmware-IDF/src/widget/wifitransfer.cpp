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
#include "widget/wifitransfer.h"
#include "ssd1331.h"

#define X_ORIGIN 0
#define Y_ORIGIN 16

#define ICON_WIDTH 15
#define ICON_HEIGHT 25


using namespace Widget;

const unsigned char icon[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x0F, 0xC0, 0x3F, 0xF0, 0x70, 0x38, 
0xE0, 0x1C, 0xC7, 0x8C, 0x9F, 0xE4, 0x38, 0x70, 
0x20, 0x10, 0x07, 0x80, 0x0C, 0xC0, 0x08, 0x40, 
0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00};

/**
 * Construct a SampleRate widget
 */
Wifitransfer::Wifitransfer(void (*toggleWifi)()) : AbstractWidget(X_ORIGIN, Y_ORIGIN, toggleWifi) { }

/**
 * Paint the SampleRate widget icon to the screen
 * Icon shows current fix status
 */
void Wifitransfer::paintLogo()
{
        SSD1331_mono_bitmap(_xorigin + 5, _yorigin + 4, icon, ICON_WIDTH, ICON_HEIGHT, WHITE);
    
}
void Wifitransfer::setStatus(int FrequencyRate)
{
    //Update Widget
}
/**
 * Return the message that should be displayed when the widget is selected
 */
std::string Wifitransfer::getMessage()
{

    std::stringstream msgStream;

    msgStream<<"Wifi:";
    return msgStream.str();
    
}