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

#ifndef MENU_H
#define MENU_H

#include <string>
#include <sstream>

class SubMenu;
class Menu;

class MenuItem
{
    friend class SubMenu;
    friend class Menu;

public:
    MenuItem(std::string text);
    virtual ~MenuItem();

    virtual void paint(int x0, int y0, bool isSelected);
    virtual void paintSelection(int x0, int y0, bool clear, bool isSelected = false);

    void addSubmenu(SubMenu* subMenu);
    virtual void addAction(void (*action)());
    virtual void addActandSubM(void (*action)(), SubMenu* subMenu);
    virtual void action();

protected:
    SubMenu* _childSubMenu = nullptr;
    void (*_action)() = nullptr;

    std::string _text = "";

    MenuItem* _previousItem = nullptr;
    MenuItem* _nextItem = nullptr;
};

class SubMenu
{
    friend class Menu;

public:
    SubMenu(std::string text);
    SubMenu(std::string text, SubMenu* parentMenu);
    ~SubMenu();

    void paint(int x0, int y0);
    void paintSelection(int x0, int y0);

    void selectNextItem();
    void selectPreviousItem();

    void addItem(MenuItem* item);

private:
    std::string _text = "";

    MenuItem* _firstItem = nullptr;
    MenuItem* _currentItem = nullptr;
};

class Menu
{
public:
    Menu();
    ~Menu();

    void paint(int x0, int y0);
    void paintSelection(int x0, int y0);

    void action();
    void previous();
    void next();

private:
    SubMenu* _currentSubMenu = nullptr;
};
#endif
