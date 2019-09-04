/*
 * Homescreen class for Open IMU
 * Displays the widgets and move between them using previous, next and action functions
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

#include "homescreen.h"
#include "ssd1331.h"
#include <sstream>
#include <iomanip>
#include <cmath>

/**
 * Construct an Homescreen object
 */
Homescreen::Homescreen() { }

/**
 * Add a widget to the homescreen
 * Make it selected if it is the first one to be added
 * 
 * @param Widget::AbstractWidget* widget : A pointer to the widget to add
 */
void Homescreen::addWidget(Widget::AbstractWidget* widget)
{
    _widgets.push_back(widget);
    if (_widgets.size() == 1)
    {
        _currentWidget = _widgets.begin();
        (*_currentWidget)->select();
    }
}

/**
 * Make the previous widget selected
 * Do nothing if the first widget is selected
 */
void Homescreen::previous()
{
    if (_currentWidget != _widgets.begin())
    {
        (*_currentWidget)->unselect();
        _currentWidget--;
        (*_currentWidget)->select();
    }

    if (_isVisible) paint();
}

/**
 * Make the next widget selected
 * Do nothing if the last widget is selected
 */
void Homescreen::next()
{
    if (std::distance(_currentWidget, _widgets.end()) > 1)
    {
        (*_currentWidget)->unselect();
        _currentWidget++;
        (*_currentWidget)->select();
    }

    if (_isVisible) paint();
}

/**
 * Perform the selected widget action
 * Will do nothing if no action is set in the widget
 */
void Homescreen::action()
{
    (*_currentWidget)->performAction();

    if (_isVisible) paint();
}

/**
 * Paint the homescreen to the screen buffer
 */
void Homescreen::paint()
{

    SSD1331_clear();

    if(_isLogging)  // Show current log time in top bar
    {
        time_t now;
        time(&now);
        double elapsed = difftime(now, _logStart);

        std::stringstream topStream;
        topStream << std::setfill('0');
        topStream << std::setw(2);
        topStream <<  floor(elapsed / 3600) << ":";
        topStream << std::setfill('0');
        topStream << std::setw(2);
        topStream << floor(fmod(elapsed, 3600)/60) << ":";
        topStream << std::setfill('0');
        topStream << std::setw(2);
        topStream << floor(fmod(elapsed, 60)) << " / ";

        topStream << std::setfill('0');
        topStream << std::setw(2);
        topStream <<  floor(_logCapacity / 3600) << ":";
        topStream << std::setfill('0');
        topStream << std::setw(2);
        topStream << floor(fmod(_logCapacity, 3600)/60);

        SSD1331_string(0, 0, topStream.str().c_str(), 12, 1, WHITE);
    }
    
    else    // Show current date / time in top bar
    {
        time_t now;
        struct tm *timeInfo;
        time(&now);
        timeInfo = gmtime(&now);

        char strftimeBuf[64];
        strftime(strftimeBuf, sizeof(strftimeBuf), "%x %H:%M", timeInfo);
        SSD1331_string(5, 0, strftimeBuf, 12, 1, WHITE);
    }
    
    for (std::list<Widget::AbstractWidget*>::iterator i = _widgets.begin(); i != _widgets.end(); i++)
    {
        (*i)->paint();
    }

    SSD1331_display();
}

/**
 * Tells the homescreen if it should paint itself to the screen
 * 
 * @param bool isVisible : if the homescreen should be displayed
 */
void Homescreen::setVisible(bool isVisible)
{
    _isVisible = isVisible;

    for (std::list<Widget::AbstractWidget*>::iterator i = _widgets.begin(); i != _widgets.end(); i++)
    {
        (*i)->setVisible(isVisible);
    }

    if (_isVisible) paint();
}

/**
 * Place the homescreen in log state where it displays elapsed time
 * 
 * @param double logCapacity : the maximum log duration at start of log in seconds
 */
void Homescreen::startLog(double logCapacity)
{
    _isLogging = true;
    _logCapacity = logCapacity;

    time(&_logStart);
}

/**
 * Cancel the homescreen log state
 */
void Homescreen::stopLog()
{
    _isLogging = false;
}