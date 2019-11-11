/*
 * Homescreen class for Open IMU
 * Displays the widgets and move between them using previous, next and action methods
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

#include <list>

#include "widget/widget.h"

class Homescreen
{
public:
    Homescreen();

    void addWidget(Widget::AbstractWidget* widget);

    void previous();
    void next();
    void action();

    void setVisible(bool isVisible);
    bool getVisible(void);

    void startLog(double logCapacity);
    void stopLog();

    void replaceSelection(void);
    void setLogID(int id);

private:
    void paint();

    std::list<Widget::AbstractWidget*> _widgets;
    std::list<Widget::AbstractWidget*>::iterator _currentWidget;

    bool _isVisible = false;

    time_t _logStart;
    double _logCapacity;
    bool _isLogging = false;
    int _logid=0;
};