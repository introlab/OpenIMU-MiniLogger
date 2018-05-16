/**
 *  @filename   :   epdif.cpp
 *  @brief      :   Implements EPD interface functions
 *                  Users have to implement all the functions in epdif.cpp
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     August 10 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "epdif.h"
#include <SPI.h>

IOExpander EpdIf::_expander;

EpdIf::EpdIf() {
}

EpdIf::~EpdIf() {
}

bool EpdIf::isExternal(int pin) {
    return (pin == RST_PIN);
}

void EpdIf::DigitalWrite(int pin, int value) {
    if(isExternal(pin)) {
        _expander.digitalWrite(pin, value);
    }
    else {
        digitalWrite(pin, value);
    }
}

int EpdIf::DigitalRead(int pin) {
    if(isExternal(pin)) {
        return _expander.digitalRead(pin);
    }
    else {
        return digitalRead(pin);
    }
}

void EpdIf::epdPinMode(int pin, int mode) {
    if(isExternal(pin)) {
        _expander.pinMode(pin, mode);
    }
    else {
        pinMode(pin, mode);
    }
}

void EpdIf::DelayMs(unsigned int delaytime) {
    delay(delaytime);
}

void EpdIf::SpiTransfer(unsigned char data) {
    if(_expander.acquire(100))
    {
      digitalWrite(CS_PIN, LOW);
      SPI.transfer(data);
      digitalWrite(CS_PIN, HIGH);
      _expander.release();
    }
    else {
      Serial.println("unable to lock SPI bus in EpdIf::SpiTransfer");
    }
}

int EpdIf::IfInit(void) {
    _expander.begin();
    epdPinMode(CS_PIN, OUTPUT);
    epdPinMode(RST_PIN, OUTPUT);
    epdPinMode(DC_PIN, OUTPUT);
    epdPinMode(BUSY_PIN, INPUT);
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    SPI.begin();
    return 0;
}
