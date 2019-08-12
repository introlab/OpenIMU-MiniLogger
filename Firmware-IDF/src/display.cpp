
#include "display.h"
#include "fonts.h"
#include "string.h"
#include <sys/time.h>

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

void Display::test(float volts, float current)
{
    //float volts = 0;
    //float current = 0;
    bool validData = false;
    bool stateLog = false;
    bool sdLog = false;


    _blackPaint.Clear(1);
    _redPaint.Clear(1);

    char batt_text[200];
    memset(batt_text,0, 200);

    sprintf(batt_text, "B: %1.2fV, %1.3fA", volts, current);

    _blackPaint.DrawStringAt(5, 2, batt_text, &Font16, 0);

  
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%a %b %d", &timeinfo);
    _blackPaint.DrawStringAt(35, 80, strftime_buf, &Font20, 0);

    strftime(strftime_buf, sizeof(strftime_buf), "%R", &timeinfo);
    _blackPaint.DrawStringAt(47, 105, strftime_buf, &Font24, 0);

    strftime(strftime_buf, sizeof(strftime_buf), ":%S", &timeinfo);
    _blackPaint.DrawStringAt(131, 111, strftime_buf, &Font16, 0);

    std::stringstream gps_data, logstate, sdstate;

    sdstate << "  SD Mode : " ;

    (sdLog) ?  sdstate << "USB" :  sdstate << "Log";

    _blackPaint.DrawStringAt(0, 25, sdstate.str().c_str(), &Font16, 0);

    gps_data << "  GPS fix : " ;

    (validData) ?  gps_data << "Yes" :  gps_data << "No";

    _blackPaint.DrawStringAt(0, 50, gps_data.str().c_str(), &Font16, 0);
    logstate << " Logging: ";

    (stateLog) ?  logstate << "ON" :  logstate << "OFF";

    _blackPaint.DrawStringAt(0, 170, logstate.str().c_str(), &Font20, 0);


    _epd.DisplayFrame(_blackImage, NULL/*_redImage */);

/*
    float voltage = 0;
    float current = 0;
    _blackPaint.Clear(1);
    _redPaint.Clear(1);
    //_blackPaint.DrawStringAt(5, 2, "Hello World", &Font16, 0);

    char batt_text[200];
    memset(batt_text,0, 200);
    sprintf(batt_text, "B: %1.2fV, %1.3fA", voltage, current);
    _blackPaint.DrawStringAt(5, 2, batt_text, &Font16, 0);

    _epd.DisplayFrame(_blackImage, NULL);
*/
}

// display sleep screen
void Display::displayVoltage(float volts, float current,bool validData, bool stateLog, bool sdLog)
{
 
    _blackPaint.Clear(1);
    _redPaint.Clear(1);

    char batt_text[200];
    memset(batt_text,0, 200);

    sprintf(batt_text, "B: %1.2fV, %1.3fA", volts, current);

    _blackPaint.DrawStringAt(5, 2, batt_text, &Font16, 0);

  
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%a %b %d", &timeinfo);
    _blackPaint.DrawStringAt(35, 80, strftime_buf, &Font20, 0);

    strftime(strftime_buf, sizeof(strftime_buf), "%R", &timeinfo);
    _blackPaint.DrawStringAt(47, 105, strftime_buf, &Font24, 0);

    strftime(strftime_buf, sizeof(strftime_buf), ":%S", &timeinfo);
    _blackPaint.DrawStringAt(131, 111, strftime_buf, &Font16, 0);

    std::stringstream gps_data, logstate, sdstate;

    sdstate << "  SD Mode : " ;

    (sdLog) ?  sdstate << "USB" :  sdstate << "Log";

    _blackPaint.DrawStringAt(0, 25, sdstate.str().c_str(), &Font16, 0);

    gps_data << "  GPS fix : " ;

    (validData) ?  gps_data << "Yes" :  gps_data << "No";

    _blackPaint.DrawStringAt(0, 50, gps_data.str().c_str(), &Font16, 0);
    logstate << " Logging: ";

    (stateLog) ?  logstate << "ON" :  logstate << "OFF";

    _blackPaint.DrawStringAt(0, 170, logstate.str().c_str(), &Font20, 0);


    _epd.DisplayFrame(_blackImage, NULL/*_redImage */);
    

}
