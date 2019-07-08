#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "ssd1331.h"
#include "menu.h"

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
    void displayVoltage(float volts, float current,bool validData, bool stateLog, bool sdLog);

private:

    static Display* _instance;
     Display();
    ~Display();
};

#endif
