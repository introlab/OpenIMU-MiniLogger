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

#include "widget/widget.h"
#include "ssd1331.h"
#include <stdlib.h>
#include <stdio.h>

#define WIDGET_MSG_Y_ORIGIN 51

using namespace Widget;

/**
 * Construct a widget
 *
 * @param uint_8t xorigin: leftmost pixel of the widget when printed with its selection rectangle
 * @param uint_8t yorigin: topmost pixel of the widget when printed with its selection rectangle
 * @param void (*action)(): action function to perform
 */
AbstractWidget::AbstractWidget(
    uint8_t xorigin,
    uint8_t yorigin,
    void (*action)()) : _xorigin{xorigin}, _yorigin{yorigin}, _action{action} { }

/**
 * Mark this widget as selected and update the selection rectangle
 */
void AbstractWidget::select()
{
    _selected = true;
}

/**
 * Mark this widget as not selected and update the selection rectangle
 */
void AbstractWidget::unselect()
{
    _selected = false;
}

/**
 * Perform the widget action and reprint it
 * Do nothing if no action was set at contruction
 */
void AbstractWidget::performAction()
{
    if (_action != nullptr) _action();
}

/**
 * Paint the widget to the screen
 * Clear the occupied screen portion and write the display buffer is only the widget has to be displayed
 *
 * @param bool meOnly : if only the widget has to be redrawn
 */
void AbstractWidget::paint(bool meOnly)
{
    //printf("Paint Logo\n");

    if (meOnly)
    {
        SSD1331_rectangle(_xorigin, _yorigin, _xorigin + WIDGET_WIDTH - 1, _yorigin + WIDGET_HEIGHT -1 , BLACK, true);
        if (_selected) SSD1331_rectangle(0, WIDGET_MSG_Y_ORIGIN, OLED_WIDTH - 1, OLED_HEIGHT - 1, BLACK, true);
    }

    paintLogo();
    if (_selected)
    {
        paintRectangle();
        paintMessage();
    }

    if (meOnly) SSD1331_display();
}

/**
 * Tells the widget if it should paint itself to the screen
 *
 * @param bool isVisible : if the widget should be displayed
 */
void AbstractWidget::setVisible(bool isVisible)
{
    _visible = isVisible;
}

/**
 * Paint the widget's rectangle to the sreen
 */
void AbstractWidget::paintRectangle()
{
    SSD1331_rectangle(_xorigin, _yorigin, _xorigin + WIDGET_WIDTH - 1, _yorigin + WIDGET_HEIGHT, GREEN);
}

/**
 * Paint the widget's message to the screen
 */
void AbstractWidget::paintMessage()
{
    std::string message = getMessage();
    uint8_t messageOffset = abs(16 - (uint8_t) message.length()) * 3;

    SSD1331_string(messageOffset, WIDGET_MSG_Y_ORIGIN, message.c_str(), 12, 1, GREEN);
}