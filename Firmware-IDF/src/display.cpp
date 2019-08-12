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


// Paint state
void Display::displayVoltage(float volts, float current,bool validData, bool stateLog, bool sdLog)
{
    /* volts: battery voltage
    */

    SSD1331_clear();

    std::stringstream batt_text;

    batt_text << "B: " << volts << "V " << current << "A";
    SSD1331_string(5, 2, batt_text.str().c_str(), 12, 1, BLACK);
/* 
    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = gmtime(&now);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%a %b %d", timeinfo);
    _blackPaint.DrawStringAt(35, 80, strftime_buf, &Font20, 0);

    strftime(strftime_buf, sizeof(strftime_buf), "%R", timeinfo);
    _blackPaint.DrawStringAt(47, 105, strftime_buf, &Font24, 0);

    strftime(strftime_buf, sizeof(strftime_buf), ":%S", timeinfo);
    _blackPaint.DrawStringAt(131, 111, strftime_buf, &Font16, 0);

    std::stringstream gps_data, logstate, sdstate;

    sdstate << "  SD Mode : " ;

    (sdLog) ?  sdstate << "USB" :  sdstate << "Log";

    _blackPaint.DrawStringAt(0, 25, sdstate.str().c_str(), &Font16, 0);

    gps_data << "  GPS data : " ;

    (validData) ?  gps_data << "Yes" :  gps_data << "No";

    _blackPaint.DrawStringAt(0, 50, gps_data.str().c_str(), &Font16, 0);
    logstate << " Logging: ";

    (stateLog) ?  logstate << "ON" :  logstate << "OFF";

    _blackPaint.DrawStringAt(0, 170, logstate.str().c_str(), &Font20, 0);*/
}