/*
 * Base class for OPEN IMU homescreen widget
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

#pragma once

#include <stdint.h>
#include <string>

#define WIDGET_WIDTH 23
#define WIDGET_HEIGHT 31

namespace Widget
{

class AbstractWidget
{
public:
    AbstractWidget(uint8_t xorigin = 0, uint8_t yorigin = 0, void (*action)() = nullptr);

    void select();
    void unselect();
    void performAction();
    void paint(bool meOnly = false);
    void setVisible(bool isVisible);

protected:
    uint8_t _xorigin;
    uint8_t _yorigin;

    bool _selected = false;
    bool _visible = false;

    void (*_action)();

    virtual std::string getMessage() = 0;
    void paintRectangle();
    void paintMessage();
    virtual void paintLogo() = 0;
};

}