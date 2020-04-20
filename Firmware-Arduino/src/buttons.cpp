#include "buttons.h"
#include "ioexpander.h"

namespace 
{
    void readButtons(void *pvParameters) 
    {
        Buttons *buttons = reinterpret_cast<Buttons*>(pvParameters);
        assert(buttons);
        printf("readButtons Task starting...\n");

        //Initialize last tick
        TickType_t _lastTick = xTaskGetTickCount();

        while(1)
        {
            //10Hz
            vTaskDelayUntil(&_lastTick, 100 / portTICK_RATE_MS);

            buttons->read();
        }

    }
}


Buttons* Buttons::_instance = NULL;

Buttons* Buttons::instance()
{
    if (Buttons::_instance == NULL)
        Buttons::_instance = new Buttons();
    return Buttons::_instance;
}

Buttons::Buttons()
{
    _mutex = xSemaphoreCreateMutex();
    assert(_mutex != NULL);

    //Setup pins
    IOExpander::instance()->pinMode(EXT_PIN02_BUTTON0, INPUT);
    IOExpander::instance()->pullupMode(EXT_PIN02_BUTTON0, HIGH);
    IOExpander::instance()->pinMode(EXT_PIN06_BUTTON1, INPUT);
    IOExpander::instance()->pullupMode(EXT_PIN06_BUTTON1, HIGH);
    IOExpander::instance()->pinMode(EXT_PIN08_BUTTON2, INPUT);
    IOExpander::instance()->pullupMode(EXT_PIN08_BUTTON2, HIGH);
    IOExpander::instance()->pinMode(EXT_PIN09_BUTTON3, INPUT);
    IOExpander::instance()->pullupMode(EXT_PIN09_BUTTON3, HIGH);

    //reset state
    reset();

    xTaskCreate(&readButtons, "Buttons", 2048, this, 15, &_buttonsTaskHandle);
}

Buttons::~Buttons()
{

}

int Buttons::getActionCtn()
{
    lock();
    int val = _actionCtn;
    unlock();
    return val;
}

int Buttons::getPreviousCtn()
{
    lock();
    int val = _previousCtn;
    unlock();
    return val;
}

int Buttons::getNextCtn()
{   
    lock();
    int val = _nextCtn;
    unlock();
    return val;
}

int Buttons::getBackCtn()
{
    lock();
    int val = _backCtn;
    unlock();
    return val;
}

void Buttons::decrementActionCtn()
{
    lock();
    _actionCtn--;
    unlock();
}

void Buttons::decrementPreviousCtn()
{
    lock();
    _previousCtn--;
    unlock();
}

void Buttons::decrementNextCtn()
{
    lock();
    _nextCtn--;
    unlock();
}

void Buttons::decrementBackCtn()
{   
    lock();
    _backCtn--;
    unlock();
}

void Buttons::reset()
{
    lock();
    _actionCtn = 0;
    _previousCtn = 0;
    _nextCtn = 0;
    _backCtn = 0;

    _lastAction = false;
    _lastPrevious = false;
    _lastNext = false;
    _lastBack = false;
    unlock();
}

void Buttons::lock()
{
    assert(xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE);
}

void Buttons::unlock()
{
    assert(xSemaphoreGive(_mutex) == pdTRUE);
}

void Buttons::read()
{
    //WARNING this will be called from buttonsTask
    //Read button states
    bool next = IOExpander::instance()->digitalRead(EXT_PIN06_BUTTON1) == 0;
    bool action = IOExpander::instance()->digitalRead(EXT_PIN08_BUTTON2) == 0;
    bool previous = IOExpander::instance()->digitalRead(EXT_PIN02_BUTTON0) == 0;
    bool back = IOExpander::instance()->digitalRead(EXT_PIN09_BUTTON3) == 0;

    lock();
    if(action && action != _lastAction) 
    {
        _actionCtn++;
        printf("Action BTN\n");
    }

    if(previous && previous != _lastPrevious) 
    {
        _previousCtn++;
        printf("Previous BTN\n");
    }

    if(next && next != _lastNext) {
        _nextCtn++;
        printf("Next BTN\n");
    }

    if(back && back != _lastBack) 
    {
        _backCtn++;
        printf("Back BTN\n");
    }

    _lastAction = action;
    _lastPrevious = previous;
    _lastNext = next;
    _lastBack = back;

    unlock();
}