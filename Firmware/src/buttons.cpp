#include "buttons.h"
#include  <sstream>

namespace
{
    // Flags for buttons read
    TickType_t _lastButtonRead;

    int _actionCtn = 0;
    int _previousCtn = 0;
    int _nextCtn = 0;

    bool _lastAction = false;
    bool _lastPrevious = false;
    bool _lastNext = false;
    void readButton(void *pvParameters);
}

Buttons::Buttons()
{

}

Buttons::~Buttons()
{

}

void Buttons::begin()
{
    // Enable IO expander
    ioExpander.pinMode(EXT_PIN11_BUTTON0, INPUT);
    ioExpander.pullupMode(EXT_PIN11_BUTTON0, HIGH);
    ioExpander.pinMode(EXT_PIN06_BUTTON1, INPUT);
    ioExpander.pullupMode(EXT_PIN06_BUTTON1, HIGH);
    ioExpander.pinMode(EXT_PIN08_BUTTON2, INPUT);
    ioExpander.pullupMode(EXT_PIN08_BUTTON2, HIGH);
    ioExpander.pinMode(EXT_PIN09_BUTTON3, INPUT);
    ioExpander.pullupMode(EXT_PIN09_BUTTON3, HIGH);

    //USe
    //delay(200);

    xTaskCreate(&readButton, "Buttons", 2048, NULL, 3, NULL);
}

namespace {

    void readButton(void *pvParameters) {
        bool action, previous, next;

        _lastButtonRead = xTaskGetTickCount();

        Serial.println("readButton task starting...");

        while(1) {
            vTaskDelayUntil(&_lastButtonRead, 100 / portTICK_RATE_MS);

            previous = ioExpander.digitalRead(EXT_PIN06_BUTTON1) != 0;
            action = ioExpander.digitalRead(EXT_PIN08_BUTTON2) != 0;
            next = ioExpander.digitalRead(EXT_PIN11_BUTTON0) != 0;
            //Serial.printf("state: %i %i %i \n",previous,action,next);

            if(action && action != _lastAction) {
                _actionCtn++;
            }

            if(previous && previous != _lastPrevious) {
                _previousCtn++;
            }

            if(next && next != _lastNext) {
                _nextCtn++;
            }

            _lastAction = action;
            _lastPrevious = previous;
            _lastNext = next;
        }
    }
}

int Buttons::getActionCtn()
{
    return _actionCtn;
}

int Buttons::getPreviousCtn()
{
    return _previousCtn;
}

int Buttons::getNextCtn()
{
    return _nextCtn;
}

void Buttons::decrementActionCtn()
{
    _actionCtn--;
}

void Buttons::decrementPreviousCtn()
{
    _previousCtn--;
}

void Buttons::decrementNextCtn()
{
    _nextCtn--;
}
