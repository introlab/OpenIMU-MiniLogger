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
#include "spibus.h"
#include <string.h>

//Anonymous namespace
namespace
{

    void cs_active(spi_transaction_t* trans)
    {
        gpio_set_level((gpio_num_t)CS_PIN, 0);
    }

    void cs_inactive(spi_transaction_t* trans)
    {
        gpio_set_level((gpio_num_t)CS_PIN, 1);
    }



}

spi_device_handle_t EpdIf::_handle;
spi_device_interface_config_t EpdIf::_config;

EpdIf::EpdIf() {
    //SPI device interface configuration
    memset(&_config, 0, sizeof(spi_device_interface_config_t));
    _config.command_bits = 8;
    _config.address_bits = 0;
    _config.dummy_bits = 0;
    _config.mode = 0;
    _config.duty_cycle_pos = 128;  // default 128 = 50%/50% duty
    _config.cs_ena_pretrans = 0;  // 0 not used
    _config.cs_ena_posttrans = 0;  // 0 not used
    _config.clock_speed_hz = 10000000;
    _config.spics_io_num = -1;
    _config.flags = 0;  // 0 not used
    _config.queue_size = 1;
    _config.pre_cb = cs_active;
    _config.post_cb = cs_inactive;

    esp_err_t ret = spi_bus_add_device(HSPI_HOST, &_config, &_handle);
    printf("EpdIf::EpdIf() SPI BUS ADD DEVICE RET: %i handle: %p\n", ret, _handle);
    assert(ret == ESP_OK);
}

EpdIf::~EpdIf() {
}

bool EpdIf::isExternal(int pin) {
    return (pin == RST_PIN);
}

void EpdIf::DigitalWrite(int pin, int value) {
    //printf("EpdIf::DigitalWrite %i %i\n", pin, value );
    if(isExternal(pin)) {
        //Serial.printf("EpdIf::DigitalWrite (ext) %i %i\n", pin, value );
        IOExpander::instance().digitalWrite(pin, value);
    }
    else {
        gpio_set_level((gpio_num_t)pin, value);
    }
}

int EpdIf::DigitalRead(int pin) {
    //printf("EpdIf::DigitalRead %i\n", pin);
    if(isExternal(pin)) {
        return IOExpander::instance().digitalRead(pin);
    }
    else {
        return gpio_get_level((gpio_num_t) pin);
    }
}

void EpdIf::epdPinMode(int pin, int mode) {
    //printf("EpdIf::epdPinMode %i %i\n", pin, mode);
    if(isExternal(pin)) {
        IOExpander::instance().pinMode(pin, mode);
    }
    else {

        gpio_config_t io_conf;
        //No interrupt 
        io_conf.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_DISABLE;
        //set as input/output mode
        io_conf.mode = (gpio_mode_t) mode;
        //bit mask of the pins that you want to set,e.g.GPIO33
        io_conf.pin_bit_mask =  (1ULL << pin);
        //disable pull-down mode
        io_conf.pull_down_en = (gpio_pulldown_t) 0;
        //enable pull-up mode on input 
        if (mode == GPIO_MODE_INPUT)
            io_conf.pull_up_en = (gpio_pullup_t) 1;
        else
            io_conf.pull_up_en = (gpio_pullup_t) 0;
        //configure GPIO with the given settings
        gpio_config(&io_conf);
    }
}

void EpdIf::DelayMs(unsigned int delaytime) {
    //delay(delaytime);
    vTaskDelay(delaytime / portTICK_RATE_MS);
}

void EpdIf::SpiTransfer(unsigned char data) {
    //printf("EpdIf::SpiTransfer\n");
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.cmd = data;
    
    esp_err_t err = SPIBus::spi_device_transmit(_handle, &trans);
    if (err != ESP_OK)
        printf("EpdIf::SpiTransfer error: %i\n", err);

}

int EpdIf::IfInit(void) {
    epdPinMode(CS_PIN, GPIO_MODE_OUTPUT);
    epdPinMode(RST_PIN, GPIO_MODE_OUTPUT);
    epdPinMode(DC_PIN, GPIO_MODE_OUTPUT);
    epdPinMode(BUSY_PIN, GPIO_MODE_INPUT);
    return 0;
}
