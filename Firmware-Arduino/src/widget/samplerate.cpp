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
#include "widget/samplerate.h"
#include "ssd1331.h"

#define X_ORIGIN 0
#define Y_ORIGIN 16

#define ICON_WIDTH 15
#define ICON_HEIGHT 25


using namespace Widget;

/*const unsigned char icon[] = {
 0x06,0x00,0x06,0x00,0x0c,0x00,0x38,0x00,0x60,0x00,0xc0,0x00,
 0x60,0x00,0x38,0x00,0x0e,0x00,0x06,0x00,0x03,0x00,0x01,0xc0,
 0x00,0x60,0x00,0x30,0x00,0x60,0x01,0xc0,0x03,0x00,0x26,0x40,
 0x36,0xc0,0x1f,0x80,0x0f,0x00,0x06,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00};*/

const unsigned char icon[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0xFC, 
0x66, 0xFC, 0x66, 0x1C, 0x66, 0x18, 0x66, 0x18, 
0x66, 0x18, 0x66, 0x18, 0x66, 0x38, 0x7E, 0x30, 
0x7E, 0x30, 0x66, 0x20, 0x66, 0x60, 0x66, 0x60, 
0x66, 0x60, 0x66, 0xE0, 0x66, 0xC0, 0x66, 0xC0, 
0x66, 0xFC, 0x66, 0xFC, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00};

/**
 * Construct a SampleRate widget
 */
SampleRate::SampleRate(void (*toggleChangeRate)()) : AbstractWidget(X_ORIGIN, Y_ORIGIN, toggleChangeRate) { }

/**
 * Paint the SampleRate widget icon to the screen
 * Icon shows current fix status
 */
void SampleRate::paintLogo()
{
        SSD1331_mono_bitmap(_xorigin + 5, _yorigin + 4, icon, ICON_WIDTH, ICON_HEIGHT, WHITE);
    
}
void SampleRate::setStatus(int FrequencyRate)
{
    if(FrequencyRate==1) RateHz=10;
    if(FrequencyRate==2) RateHz=50;
    if(FrequencyRate==3) RateHz=100;
    if(FrequencyRate==4) RateHz=200;
}
/**
 * Return the message that should be displayed when the widget is selected
 */
std::string SampleRate::getMessage()
{

    std::stringstream msgStream;

    msgStream<<"Rate:"<< RateHz << "Hz" ;
    return msgStream.str();
    
}