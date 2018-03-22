#include "buttons.h"

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

    Adafruit_MCP23008 _mcp;
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
    _mcp.begin();

    _mcp.pinMode(2, INPUT);
    _mcp.pullUp(2, HIGH);

    _mcp.pinMode(6, INPUT);
    _mcp.pullUp(6, HIGH);

    _mcp.pinMode(7, INPUT);
    _mcp.pullUp(7, HIGH);

    xTaskCreate(&readButton, "Buttons", 2048, NULL, 5, NULL);
}

namespace {
    
    void readButton(void *pvParameters) {
        bool action, previous, next;

        _lastButtonRead = xTaskGetTickCount();

        while(1) {
            vTaskDelayUntil(&_lastButtonRead, 100 / portTICK_RATE_MS);
            action = _mcp.digitalRead(2) != 0;
            previous = _mcp.digitalRead(6) != 0;
            next = _mcp.digitalRead(7) != 0;

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
