#include "display.h"
#include "fonts.h"
#include <string>
#include <sstream>


Display::Display()
{

}

Display::~Display()
{

}

void Display::begin()
{
  _epd.Init();
}

void Display::end()
{
  _epd.Sleep();
}

void Display::showSplashScreen(uint64_t mac_adress)
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);

    _redPaint.DrawStringAt(10, 20, "Open IMU", &Font24, 0);
    _redPaint.DrawStringAt(147, 26, "v0.1", &Font16, 0);
    _blackPaint.DrawStringAt(10, 50, "System is idle", &Font16, 0);

    std::stringstream mac_string;
    mac_string << "MAC:" << std::uppercase << std::hex << mac_adress;  // Hex format
    // mac_string << "MAC:" << mac_adress;  // Decimal format, some devices have bigger adress than can be display at this font
    _blackPaint.DrawStringAt(10, 180, mac_string.str().c_str(), &Font16, 0);





    //_blackPaint.DrawStringAt(10, 245, "SW2 to start...", &Font16, 0);

    _epd.DisplayFrame(_blackImage, _redImage);

}

void Display::showMenu(Menu* menu)
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);
    //menu->paint(_blackPaint, _redPaint, 5, 5);
    menu->paint(_blackPaint, _blackPaint, 5, 5);
    _epd.DisplayFrame(_blackImage, NULL /*_redImage */);

}

void Display::updateMenu(Menu* menu, bool stateLog)
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);
    //menu->paint(_blackPaint, _redPaint, 5, 5);
    menu->paint(_blackPaint, _blackPaint, 5, 5);
    std::stringstream logstate;
    logstate << " Logging: ";
    (stateLog) ?  logstate << "ON" :  logstate << "OFF";
    _blackPaint.DrawStringAt(0, 170, logstate.str().c_str(), &Font20, 0);


    _epd.DisplayFrame(_blackImage, NULL /*_redImage*/);
}

void Display::clear()
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);
    _epd.DisplayFrame(_blackImage, _redImage);
}


// display sleep screen
void Display::displayVoltage(float volts, float current,bool validData, bool stateLog, bool sdLog)
{
    _blackPaint.Clear(1);
    _redPaint.Clear(1);

    std::stringstream batt_text;

    batt_text << "B: " << volts << "V " << current << "A";

    _blackPaint.DrawStringAt(5, 2, batt_text.str().c_str(), &Font16, 0);

#if 0
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

    //Serial.print("Current time ");
    //Serial.println(strftime_buf);

    std::stringstream gps_data, logstate, sdstate;



    sdstate << "  SD Mode : " ;

    (sdLog) ?  sdstate << "USB" :  sdstate << "Log";

    _blackPaint.DrawStringAt(0, 25, sdstate.str().c_str(), &Font16, 0);

    gps_data << "  GPS data : " ;

    (validData) ?  gps_data << "Yes" :  gps_data << "No";

    _blackPaint.DrawStringAt(0, 50, gps_data.str().c_str(), &Font16, 0);
    logstate << " Logging: ";
    
    (stateLog) ?  logstate << "ON" :  logstate << "OFF";

    _blackPaint.DrawStringAt(0, 170, logstate.str().c_str(), &Font20, 0);

#endif
    _epd.DisplayFrame(_blackImage, NULL/*_redImage */);

}

