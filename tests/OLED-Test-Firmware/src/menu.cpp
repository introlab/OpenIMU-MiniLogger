/*
 * Menu module for Open IMU
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

#include "menu.h"
#include "actions.h"
#include "ssd1331.h"

MenuItem::MenuItem(std::string text)
{
    _text = text;
}

MenuItem::~MenuItem()
{

}

void MenuItem::paint(int x0, int y0, bool isSelected)
{
    SSD1331_string(x0 + 3, y0 + 2, _text.c_str(), 12, 1, WHITE);
    if(isSelected) {
        SSD1331_rectangle(x0, y0, x0 + 95, y0 + 15, GREEN_CMD);
    }
}

void MenuItem::paintSelection(int x0, int y0, bool clear, bool isSelected)
{
    if(isSelected) {
        SSD1331_rectangle(x0, y0, x0 + 95, y0 + 15, GREEN_CMD);
    }
    else if(clear) {
        SSD1331_rectangle(x0, y0, x0 + 95, y0 + 15, BLACK_CMD);
    }
}

void MenuItem::addSubmenu(SubMenu* subMenu)
{
    _childSubMenu = subMenu;
    _action = nullptr;
}

void MenuItem::addAction(void (*action)())
{
    _action = action;
    _childSubMenu = nullptr;
}

void MenuItem::addActandSubM(void (*action)(), SubMenu* subMenu)
{
    _action = action;
    _childSubMenu = subMenu;
}

void MenuItem::action()
{
    if(_action != nullptr) {
        _action();
    }
}

SubMenu::SubMenu(std::string text)
{
    _text = text;
}

SubMenu::SubMenu(std::string text, SubMenu* parentMenu)
{
    _text = text;

    _firstItem = new MenuItem("Back");
    _firstItem->addSubmenu(parentMenu);

    _currentItem = _firstItem;
}

SubMenu::~SubMenu()
{

}

void SubMenu::paint(int x0, int y0)
{
    MenuItem* item = _firstItem;
    int yOffset = 18;

    SSD1331_string(x0, y0, _text.c_str(), 16, 1, GREEN);

    while(item != nullptr) {
        item->paint(x0, y0 + yOffset, item == _currentItem);
        yOffset += 15;
        item = item->_nextItem;
    }
}

void SubMenu::paintSelection(int x0, int y0)
{
    MenuItem* item = _firstItem;
    int yOffset = 18;

    while(item != nullptr) {
        item->paintSelection(x0, y0 + yOffset, true);
        yOffset += 15;
        item = item->_nextItem;
    }
    
    yOffset = 18;
    item = _firstItem;
    while(item != nullptr) {
        item->paintSelection(x0, y0 + yOffset, false, item == _currentItem);
        yOffset += 15;
        item = item->_nextItem;
    }
}

void SubMenu::selectNextItem()
{
    if(_currentItem != nullptr) {
        if(_currentItem->_nextItem != nullptr)
            _currentItem = _currentItem->_nextItem;
        else
            while(_currentItem->_previousItem != nullptr)
                _currentItem = _currentItem->_previousItem;

    }
}

void SubMenu::selectPreviousItem()
{
    if(_currentItem != nullptr) {
        if(_currentItem->_previousItem != nullptr)
            _currentItem = _currentItem->_previousItem;
        else
            while(_currentItem->_nextItem != nullptr)
                _currentItem = _currentItem->_nextItem;
    }
}

void SubMenu::addItem(MenuItem* item)
{
    if(_firstItem != nullptr) {
        item->_nextItem = _firstItem;
        _firstItem->_previousItem = item;
    }

    _firstItem = item;
    _currentItem = _firstItem;
}

Menu::Menu()
{
    MenuItem* item;
    SubMenu* subMenu;

    _currentSubMenu = new SubMenu("Main menu");

    subMenu = new SubMenu("Shutdown", _currentSubMenu);

    item = new MenuItem("Confirm");
    subMenu->addItem(item);

    item = new MenuItem("Shutdown");
    item->addSubmenu(subMenu);
    _currentSubMenu->addItem(item);


    subMenu = new SubMenu("SD Card", _currentSubMenu);

    item = new MenuItem("ESP 32");
    item->addSubmenu(_currentSubMenu);
    subMenu->addItem(item);

    item = new MenuItem("PC");
    item->addSubmenu(_currentSubMenu);
    subMenu->addItem(item);

    item = new MenuItem("SD Card");
    item->addSubmenu(subMenu);
    _currentSubMenu->addItem(item);

    subMenu = new SubMenu("Start/Stop ", _currentSubMenu);

    item = new MenuItem("Confirm");
    item->addSubmenu(_currentSubMenu);
    subMenu->addItem(item);

    item = new MenuItem("Start/Stop Log");
    item->addSubmenu(subMenu);
    _currentSubMenu->addItem(item);

}

Menu::~Menu()
{

}

void Menu::paint(int x0, int y0)
{
    _currentSubMenu->paint(x0, y0);
}

void Menu::paintSelection(int x0, int y0)
{
    _currentSubMenu->paintSelection(x0, y0);
}

void Menu::action()
{
    if(_currentSubMenu->_currentItem != nullptr) {

        if(_currentSubMenu->_currentItem->_action != nullptr )
             _currentSubMenu->_currentItem->action();

        if (_currentSubMenu->_currentItem->_childSubMenu != nullptr)
            _currentSubMenu = _currentSubMenu->_currentItem->_childSubMenu;

    }
}

void Menu::previous()
{
    _currentSubMenu->selectPreviousItem();
}

void Menu::next()
{
    _currentSubMenu->selectNextItem();
}