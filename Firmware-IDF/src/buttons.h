#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"

class Buttons
{
public:
    
    static Buttons* instance();
    int getActionCtn();
    int getPreviousCtn();
    int getNextCtn();
    int getBackCtn();


    void decrementActionCtn();
    void decrementPreviousCtn();
    void decrementNextCtn();
    void decrementBackCtn();
    
    void reset();

    void read();
private:
    Buttons();
    virtual ~Buttons();
    void lock();
    void unlock();
    static Buttons* _instance;
    int _actionCtn;
    int _previousCtn;
    int _nextCtn;
    int _backCtn;
    bool _lastAction;
    bool _lastPrevious;
    bool _lastNext;
    bool _lastBack;
    TaskHandle_t _buttonsTaskHandle;
    SemaphoreHandle_t _mutex;

};


#endif
