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
#include <stdio.h>

/**
 * Construct an Homescreen object
 */
Homescreen::Homescreen() { }

/**
 * Add a widget to the homescreen
 * Make it selected if it is the first one to be added
 * 
 * If the page has 4 widgets, a new page is created when adding a new widget.
 * 
 * @param Widget::AbstractWidget* widget : A pointer to the widget to add
 */
void Homescreen::addWidget(Widget::AbstractWidget* widget)
{
    if(_pages.empty())
    {
        std::list<Widget::AbstractWidget*> _widgets;
        _widgets.push_back(widget);
        _pages.push_back(_widgets);

        _currentPage = _pages.begin();
        _currentWidget = _currentPage->begin();
        (*_currentWidget)->select();

        
    }
    else
    {  
        if(_currentPage->size()==4)
        {
            //Page Full, creating a new page
            std::list<Widget::AbstractWidget*> _widgets;
            _widgets.push_back(widget);
            _pages.push_back(_widgets);
            _currentPage++;
        }
        else
        {
            //Add widget to the list
            _currentPage->push_back(widget);
            //printf("Adding new widget to page:%d, size:%d\n",_pages.size(),_currentPage->size());
        }

    }
}

/**
 * Make the previous widget selected
 * If previous is used on the first widget of the page, it switch pages if there's one available. Does nothing otherwise.
 * 
 */
void Homescreen::previous()
{

    //If at the first widget of the page with a page available before the actual one
    if((((std::distance(_currentWidget, _currentPage->end()))-(_currentPage->size()))==0)&&(((std::distance(_currentPage, _pages.end())-(_pages.size()))>0)))
    {
        (*_currentWidget)->unselect();
        _currentPage--;
        _currentWidget = _currentPage->end();
        _currentWidget--;
        (*_currentWidget)->select();
        printf("Previous Page \n");
    }
    //If not at the begging of the page
    else if(((_currentPage->size())-(std::distance(_currentWidget, _currentPage->end())))>0)
    {
        printf("Previous widget\n");
        (*_currentWidget)->unselect();
        _currentWidget--;
        (*_currentWidget)->select();
        
    }
    else
    {
        printf("No more widget Available\n");
    }
    
    //printf("Previous: Page Number:%d\n",std::distance(_currentPage,_pages.end()));
    //printf("Previous: Widget Number: %d\n",std::distance(_currentWidget, _currentPage->end()));

    setVisible(true);
}

/**
 * Make the next widget selected
 * If next is used on the last widget of the page, it switch pages if there's one available. Does nothing otherwise.
 */
void Homescreen::next()
{

    if (((std::distance(_currentWidget,_currentPage->end()))==1) && ((std::distance(_currentPage, _pages.end()))>1))
    {
        (*_currentWidget)->unselect();
        _currentPage++;
        _currentWidget = _currentPage->begin();
        (*_currentWidget)->select();
        //printf("Next Page \n");
    }
    else if ((std::distance(_currentWidget, _currentPage->end()))>1)
    {
        //printf("Next widget\n");
        (*_currentWidget)->unselect();
        _currentWidget++;
        (*_currentWidget)->select();
    }
    else
    {
        printf("No more widget Available\n");
    }

    //printf("Next: Page Number:%d\n",std::distance(_currentPage,_pages.end()));
    //printf("Next: Widget Number: %d\n",std::distance(_currentWidget, _currentPage->end()));

    setVisible(true);
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
        topStream << floor(fmod(elapsed, 60));
    
        topStream << std::setfill('0');
        topStream << std::setw(2);
        topStream << "   ID:" << floor(_logid);
        //topStream << std::setfill('0');
        //topStream << std::setw(2);
        //topStream << floor(fmod(_logCapacity, 3600)/60);
        
        SSD1331_string(0, 0, topStream.str().c_str(), 12, 1, WHITE);
    }
    
    else    // Show current date / time in top bar
    {
        time_t now;
        struct tm *timeInfo;
        time(&now);
        timeInfo = localtime(&now);

        char strftimeBuf[64];
        strftime(strftimeBuf, sizeof(strftimeBuf), "%d/%m/%y %H:%M", timeInfo);

        //Page string
        std::stringstream strfpageBuf;
        strfpageBuf<<(_pages.size()-(std::distance(_currentPage,_pages.end()))+1);
        std::string Pagestring = strfpageBuf.str();

        SSD1331_string(0, 0, strftimeBuf, 12, 1, WHITE);
        SSD1331_string(90, 0, Pagestring.c_str(), 12, 1, GOLDEN);
    }
    
    for (std::list<Widget::AbstractWidget*>::iterator i = _currentPage->begin(); i != _currentPage->end(); i++)
    {
        (*i)->paint();
    }

    SSD1331_display();
}

/**
 * Tells the homescreen if it should paint itself to the screen or update 
 * the widget if called from the next or previous function.
 * 
 * @param bool isVisible : if the homescreen should be displayed
 */
void Homescreen::setVisible(bool isVisible)
{
    _isVisible = isVisible;
    
    //Set invisible the widgets that are not on the current page and set visible the ones that are.
    for (std::list<std::list<Widget::AbstractWidget*>>::iterator k = _pages.begin(); k != _pages.end();k++)
    {
        if(k!=_currentPage)
        {
            for (std::list<Widget::AbstractWidget*>::iterator i = k->begin(); i != k->end(); i++)
            {
                (*i)->setVisible(false);
            }  
        }
        else
        {
            for (std::list<Widget::AbstractWidget*>::iterator i = k->begin(); i != k->end(); i++)
            {
                (*i)->setVisible(isVisible);
            }
        }
          
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

bool Homescreen::getVisible()
{
    return _isVisible;
}

//Replace the cursor of the homescreen at the beginning
void Homescreen::replaceSelection()
{ 
    for (std::list<std::list<Widget::AbstractWidget*>>::iterator k = _pages.begin(); k != _pages.end();k++)
    {
        for (std::list<Widget::AbstractWidget*>::iterator i = k->begin(); i != k->end(); i++)
        {
            (*i)->unselect();
        }   
    }
    _currentPage = _pages.begin();
    _currentWidget = _currentPage->begin();
    (*_currentWidget)->select();
    
}

void Homescreen::setLogID(int id)
{
    _logid = id;
}