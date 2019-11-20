/*
 * Test firmware for OLED screen driver development
 * It is designed to run on the Sparkfun EPS32 Thing board
 * IT WILL NOT RUN AS IS ON THE OPEN IMU HARDWARE
 * author: Cedric Godin
 * 
 * Copyright 2019 IntRoLab
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include "defines.h"
#include "spibus.h"
#include "menu.h"
#include "display.h"
#include "widget/battery.h"
#include "widget/gps.h"
#include "widget/log.h"
#include "widget/sd.h"
#include "homescreen.h"

// Module name for debuging
static const char* TAG = "main";

/*
 * Blink the ESP32 Thing LED to show ESP32 is alive
 */
void blink_led(void *pvParamters)
{
    // Init lastTick to current time
    TickType_t lastTick = xTaskGetTickCount();

    // Blink forever
    uint8_t state = 0;
    while(1)
    {
        vTaskDelayUntil(&lastTick, 1000);
        gpio_set_level((gpio_num_t)LED_PIN, state);
        state = !state;
    }
}

/*
 * Prepare ESP32 hardware for tasks
 */
void hardware_setup()
{
    // Setup alive LED
    gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction((gpio_num_t)LED_PIN, GPIO_MODE_OUTPUT);

    vTaskDelay(500 / portTICK_RATE_MS);

    // Setup SPI bus
    gpio_install_isr_service(0);
    SPIBus spi_bus;
}

/*
 * Main app
 * Demonstrate operation off the OLED screen
 */
extern "C"
{
    void app_main()
    {
        // Setup hardware
        hardware_setup();

        // State that the ESP32 is alive
        TaskHandle_t blink_led_task;
        xTaskCreate(&blink_led, "Blink", 2048, NULL, 1, &blink_led_task);
        ESP_LOGI(TAG, "Ready and blinking");

        // Boot up display
        Display* display = Display::instance();
        display->begin();

        Widget::Battery batteryWidget;
        batteryWidget.updateValue(5.0, 0.0, true);

        Widget::GPS gpsWidget;
        gpsWidget.setStatus(false);

        Widget::Log logWidget(nullptr);
        logWidget.setStatus(false);

        Widget::SD sdWidget(nullptr);
        sdWidget.setStatus(false);

        Homescreen home;
        home.addWidget(&batteryWidget);
        home.addWidget(&gpsWidget);
        home.addWidget(&logWidget);
        home.addWidget(&sdWidget);

        home.setVisible(true);

        vTaskDelay(3000 / portTICK_RATE_MS);

        home.next();
        vTaskDelay(1200);

        home.next();
        vTaskDelay(1200);

        home.startLog(12543);
        logWidget.setStatus(true);

        vTaskDelay(1200);
        home.previous();

        vTaskDelay(1200);
        home.previous();

        while(1)
        {
            vTaskDelay(500);
            home.setVisible(true);

            /*vTaskDelay(1000 / portTICK_RATE_MS);
            home.next();

            vTaskDelay(1000 / portTICK_RATE_MS);
            home.next();

            vTaskDelay(1000 / portTICK_RATE_MS);
            logWidget.setStatus(true);

            vTaskDelay(1000 / portTICK_RATE_MS);
            home.next();

            vTaskDelay(1000 / portTICK_RATE_MS);
            logWidget.setStatus(false);

            vTaskDelay(1000 / portTICK_RATE_MS);
            home.previous();

            vTaskDelay(1000 / portTICK_RATE_MS);
            home.previous();

            vTaskDelay(1000 / portTICK_RATE_MS);
            home.previous();*/
        }
    }
}