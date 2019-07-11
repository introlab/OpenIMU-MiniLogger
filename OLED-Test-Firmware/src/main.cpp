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
#include "ssd1331.h"
#include "menu.h"
#include "display.h"
#include "icons.h"

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
        vTaskDelayUntil(&lastTick, 500 / portTICK_RATE_MS);
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

        Display* display = Display::instance();
        display->begin();

        display->clear();
        SSD1331_string(6, 0, "09/03/09 12:12", 12, 1, WHITE);
        SSD1331_mono_bitmap(7, 19, battery_icon, BATTERY_ICON_WIDTH, BATTERY_ICON_HEIGHT, WHITE);
        SSD1331_mono_bitmap(29, 18, gps_icon, GPS_ICON_WIDTH, GPS_ICON_HEIGHT, RED);
        SSD1331_line(27, 17, 42, 45, RED_CMD);
        SSD1331_line(27, 45, 42, 17, RED_CMD);
        SSD1331_mono_bitmap(50, 23, logging_icon, LOGGING_ICON_WIDTH, LOGGING_ICON_HEIGHT, WHITE);
        SSD1331_mono_bitmap(73, 20, sd_icon, SD_ICON_WIDTH, SD_ICON_HEIGHT, WHITE);
        SSD1331_rectangle(48, 16, 71, 49, GREEN_CMD);
        SSD1331_string(6, 51, "Start/Stop Log", 12, 1, GREEN);
    }
}