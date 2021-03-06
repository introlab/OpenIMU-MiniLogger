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

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "ssd1331.h"
#include "menu.h"
#include "homescreen.h"

class Display
{
    public:

    //Singleton pattern
    static Display* instance();

    void begin();
    void end();


    void showSplashScreen(uint64_t mac_adress);
    void clear();
    void showMenu(Menu* menu);
    void updateMenu(Menu* menu, bool stateLog);
    void updateMenuSelection(Menu* menu, bool stateLog);
    void displayVoltage(float volts, float current,bool validData, bool stateLog, bool sdLog);

private:

    static Display* _instance;
     Display();
    ~Display();
};

#endif
