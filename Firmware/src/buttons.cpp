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
    mcp23s17.pinMode(EXT_PIN11_BUTTON0, INPUT);
    mcp23s17.pullupMode(EXT_PIN11_BUTTON0, HIGH);
    mcp23s17.pinMode(EXT_PIN06_BUTTON1, INPUT);
    mcp23s17.pullupMode(EXT_PIN06_BUTTON1, HIGH);
    mcp23s17.pinMode(EXT_PIN08_BUTTON2, INPUT);
    mcp23s17.pullupMode(EXT_PIN08_BUTTON2, HIGH);
    mcp23s17.pinMode(EXT_PIN09_BUTTON3, INPUT);
    mcp23s17.pullupMode(EXT_PIN09_BUTTON3, HIGH);

    delay(200);

    xTaskCreate(&readButton, "Buttons", 2048, NULL, 5, NULL);
}

namespace {

    void readButton(void *pvParameters) {
        bool action, previous, next;

        _lastButtonRead = xTaskGetTickCount();

        Serial.println("readButton task starting...");

        while(1) {
            vTaskDelayUntil(&_lastButtonRead, 100 / portTICK_RATE_MS);

            previous = mcp23s17.digitalRead(EXT_PIN11_BUTTON0) != 0;
            action = mcp23s17.digitalRead(EXT_PIN08_BUTTON2) != 0;
            next = mcp23s17.digitalRead(EXT_PIN06_BUTTON1) != 0;
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