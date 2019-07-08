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

    vTaskDelay(500 / portTICK_RATE_MS);

    // Setup display
    SSD1331_begin();
    SSD1331_clear();
    SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, BLUE);
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

        while (1)
        {
            SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, GRAY);
            SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, WHITE);
            SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, RED);
            SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, PINK);
            SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, YELLOW);
            SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, GOLDEN);
            SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, BROWN);
            SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, BLUE);
            SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, CYAN);
            SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, GREEN);
            SSD1331_mono_bitmap(0, 0, waveshare_logo, 96, 64, PURPLE);
        }
        
    }
}