/*
 * Display module for Open IMU
 * author: Cedric Godin
 * 
 * Copyright 2018 IntRoLab
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

#include "display.h"
#include <string>
#include <sstream>

Display* Display::_instance = NULL;

Display* Display::instance()
{
    if (Display::_instance == NULL)
        Display::_instance = new Display();
    return Display::_instance;
}

Display::Display()
{

}

Display::~Display()
{

}

void Display::begin()
{
    SSD1331_begin();
    SSD1331_clear();
}

void Display::end()
{
    SSD1331_shutdown();
}

void Display::showSplashScreen(uint64_t mac_adress)
{
    SSD1331_clear();
    SSD1331_string(0, 0, "Open IMU 0.2", 16, 1, GREEN);
    SSD1331_string(0, 20, "System is idle", 12, 1, WHITE);

    std::stringstream mac_string;
    mac_string << "MAC:" << std::uppercase << std::hex << mac_adress;

    SSD1331_string(0, 50, mac_string.str().c_str(), 12, 1, GRAY);
    SSD1331_display();
}

void Display::clear()
{
    SSD1331_clear();
}

void Display::setBrightness(Brigthness brightness)
{
    SSD1331_command(brightness);
}