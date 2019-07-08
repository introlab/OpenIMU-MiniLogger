#include "display.h"
#include <string>
#include <sstream>

Display* Display::_instance = NULL;

Display* Display::instance()
{
    if (Display::_instance == NULL)
        Display::_instance = new Display();
    return Display::_instance;
}

Display::Display()
{

}

Display::~Display()
{

}

void Display::begin()
{
    SSD1331_begin();
    SSD1331_clear();
}

void Display::end()
{
    SSD1331_shutdown();
}

void Display::showSplashScreen(uint64_t mac_adress)
{
    SSD1331_clear();
    SSD1331_string(0, 0, "Open IMU 0.1", 16, 1, GREEN);
    SSD1331_string(0, 20, "System is idle", 12, 1, WHITE);

    std::stringstream mac_string;
    mac_string << "MAC:" << std::uppercase << std::hex << mac_adress;

    SSD1331_string(0, 50, mac_string.str().c_str(), 12, 1, GRAY);
}

void Display::showMenu(Menu* menu)
{
    SSD1331_clear();
    menu->paint(5, 5);
}

void Display::updateMenu(Menu* menu, bool stateLog)
{
    SSD1331_clear();
    menu->paint(5, 5);
    std::stringstream logstate;
    logstate << " Logging: ";
    (stateLog) ?  logstate << "ON" :  logstate << "OFF";
    SSD1331_string(0, 86, logstate.str().c_str(), 12, 1, WHITE);
}

void Display::clear()
{
    SSD1331_clear();
}


// display sleep screen
void Display::displayVoltage(float volts, float current,bool validData, bool stateLog, bool sdLog)
{
    SSD1331_clear();

    std::stringstream batt_text;

    batt_text << "B: " << volts << "V " << current << "A";
    SSD1331_string(5, 2, batt_text.str().c_str(), 12, 1, BLACK);
/* 
    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = gmtime(&now);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%a %b %d", timeinfo);
    _blackPaint.DrawStringAt(35, 80, strftime_buf, &Font20, 0);

    strftime(strftime_buf, sizeof(strftime_buf), "%R", timeinfo);
    _blackPaint.DrawStringAt(47, 105, strftime_buf, &Font24, 0);

    strftime(strftime_buf, sizeof(strftime_buf), ":%S", timeinfo);
    _blackPaint.DrawStringAt(131, 111, strftime_buf, &Font16, 0);

    std::stringstream gps_data, logstate, sdstate;

    sdstate << "  SD Mode : " ;

    (sdLog) ?  sdstate << "USB" :  sdstate << "Log";

    _blackPaint.DrawStringAt(0, 25, sdstate.str().c_str(), &Font16, 0);

    gps_data << "  GPS data : " ;

    (validData) ?  gps_data << "Yes" :  gps_data << "No";

    _blackPaint.DrawStringAt(0, 50, gps_data.str().c_str(), &Font16, 0);
    logstate << " Logging: ";

    (stateLog) ?  logstate << "ON" :  logstate << "OFF";

    _blackPaint.DrawStringAt(0, 170, logstate.str().c_str(), &Font20, 0);*/
}