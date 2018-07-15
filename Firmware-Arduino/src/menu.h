#ifndef MENU_H
#define MENU_H

#include <string>
#include <sstream>

#include "epdpaint.h"

class SubMenu;
class Menu;

class MenuItem
{
    friend class SubMenu;
    friend class Menu;

public:
    MenuItem(std::string text);
    virtual ~MenuItem();

    virtual void paint(Paint &blackPaint, Paint &redPaint, int x0, int y0, bool isSelected);

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

    void paint(Paint &blackPaint, Paint &redPaint, int x0, int y0);
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

    void paint(Paint &blackPaint, Paint &redPaint, int x0, int y0);

    void action();
    void previous();
    void next();

private:
    SubMenu* _currentSubMenu = nullptr;
};

#endif
