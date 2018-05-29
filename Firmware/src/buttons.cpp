#include "buttons.h"
#include  <sstream>

namespace
{
    // Flags for buttons read
    TickType_t _lastButtonRead;

    int _actionCtn = 0;
    int _previousCtn = 0;
    int _nextCtn = 0;
    int _backCtn = 0;


    int count=0;

    bool _lastAction = false;
    bool _lastPrevious = false;
    bool _lastNext = false;
    bool _lastBack = false;
    bool _shutdown = false;
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
        bool action, previous, next, back;

        _lastButtonRead = xTaskGetTickCount();

        Serial.println("readButton task starting...");

        while(1) {
            vTaskDelayUntil(&_lastButtonRead, 100 / portTICK_RATE_MS);

            //Menu buttons
            previous = ioExpander.digitalRead(EXT_PIN06_BUTTON1) == 0;
            action = ioExpander.digitalRead(EXT_PIN08_BUTTON2) == 0;
            next = ioExpander.digitalRead(EXT_PIN11_BUTTON0) == 0;
            back = ioExpander.digitalRead(EXT_PIN09_BUTTON3) == 0;
            //Serial.printf("state: %i %i %i \n",previous,action,next);

            if(action && action != _lastAction) {
                _actionCtn++;
                Serial.println("Action BTN");
            }

            if(previous && previous != _lastPrevious) {
                _previousCtn++;
                Serial.println("Previous BTN");
            }

            if(next && next != _lastNext) {
                _nextCtn++;
                count=0;
                Serial.println("Next BTN");
            }

            // Shutdown on bottom button
            if(next && next == _lastNext) {
                count++;
                //Serial.printf("Power button %d...",count);
                if (count == 15)
                {
                    Serial.println("Shutdown...");
                    _shutdown=true;
                }
            }

            if(back && back != _lastBack) {
                _backCtn++;
                Serial.println("Back BTN");
            }

            _lastAction = action;
            _lastPrevious = previous;
            _lastNext = next;
            _lastBack = back;
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

int Buttons::getBackCtn()
{
    return _backCtn;
}

bool Buttons::getShutDown()
{
    return _shutdown;
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

void Buttons::decrementBackCtn()
{
    _backCtn--;
}

void Buttons::reset()
{
    _actionCtn = 0;
    _previousCtn = 0;
    _nextCtn = 0;
    _backCtn = 0;

    _lastAction = false;
    _lastPrevious = false;
    _lastNext = false;
    _lastBack = false;
}
