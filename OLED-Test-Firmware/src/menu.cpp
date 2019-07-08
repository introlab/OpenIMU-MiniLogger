
#include "menu.h"
#include "actions.h"

MenuItem::MenuItem(std::string text)
{
    _text = text;
}

MenuItem::~MenuItem()
{

}

void MenuItem::paint(Paint &blackPaint, Paint &redPaint, int x0, int y0, bool isSelected)
{
    blackPaint.DrawStringAt(x0 + 3, y0, _text.c_str(), &Font16, 0);
    if(isSelected) {
        redPaint.DrawRectangle(x0, y0 -4, x0 + 164, y0 + 18, 0);
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

void SubMenu::paint(Paint &blackPaint, Paint &redPaint, int x0, int y0)
{
    MenuItem* item = _firstItem;
    int yOffset = 30;

    redPaint.DrawStringAt(x0, y0, _text.c_str(), &Font24, 0);

    while(item != nullptr) {
        item->paint(blackPaint, redPaint, x0, y0 + yOffset, item == _currentItem);
        yOffset += 25;
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
    item->addAction(Actions::Shutdown);
    subMenu->addItem(item);

    item = new MenuItem("Shutdown");
    item->addSubmenu(subMenu);
    _currentSubMenu->addItem(item);


    subMenu = new SubMenu("SD Card", _currentSubMenu);

    item = new MenuItem("ESP 32");
    item->addActandSubM(Actions::SDToESP32, _currentSubMenu);
    subMenu->addItem(item);

    item = new MenuItem("PC");
    item->addActandSubM(Actions::SDToExternal, _currentSubMenu);
    subMenu->addItem(item);

    item = new MenuItem("SD Card");
    item->addSubmenu(subMenu);
    _currentSubMenu->addItem(item);

    subMenu = new SubMenu("Start/Stop ", _currentSubMenu);

    item = new MenuItem("Confirm");
    item->addActandSubM(Actions::IMUStartSD, _currentSubMenu);
    subMenu->addItem(item);

    item = new MenuItem("Start/Stop Log");
    item->addSubmenu(subMenu);
    _currentSubMenu->addItem(item);

}

Menu::~Menu()
{

}

void Menu::paint(Paint &blackPaint, Paint &redPaint, int x0, int y0)
{
    _currentSubMenu->paint(blackPaint, redPaint, x0, y0);
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
