/*****************************************************************************
*
* File                : ssd1331.c
* Hardware Environment: Raspberry Pi
* Build Environment   : GCC
* Version             : V1.0.7
* Author              : Yehui
*
*              (c) Copyright 2005-2017, WaveShare
*                   http://www.waveshare.com
*                   http://www.waveshare.net   
*                      All Rights Reserved
*              
******************************************************************************/

#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "spibus.h"
#include "ssd1331.h"
#include "defines.h"
#include "ioexpander.h"

unsigned char buffer[OLED_WIDTH * OLED_HEIGHT * 2];

// Module name for debuging
static const char* TAG = "ssd131";

// SPI device handle
static spi_device_handle_t _spi_handle = NULL;

void SPIWrite(uint8_t *buffer, int bufferLength) {
    spi_transaction_t trans = {};

    trans.tx_buffer = buffer;
    trans.length = 8 * bufferLength;

    esp_err_t ret = SPIBus::spi_device_transmit(_spi_handle, &trans);
    assert(ret == ESP_OK);
}

void SSD1331_command(uint8_t cmd) {
    gpio_set_level((gpio_num_t)PIN_NUM_DISPLAY_DC, 0);
    SPIWrite(&cmd, 1);
}

void SSD1331_beginSPI()
{
    spi_device_interface_config_t config = {};

    config.command_bits = 0;
    config.address_bits = 0;
    config.dummy_bits = 0;
    config.mode = 0;
    config.duty_cycle_pos = 128; // 50%
    config.cs_ena_pretrans = 1;
    config.cs_ena_posttrans = 2;
    config.clock_speed_hz = 10000000;
    config.spics_io_num = PIN_NUM_DISPLAY_CS;
    config.flags = 0;
    config.queue_size = 1;

    esp_err_t ret = spi_bus_add_device(HSPI_HOST, &config, &_spi_handle);
    assert(ret == ESP_OK);
    ESP_LOGI(TAG, "Added SSD1331 SPI device");
}

void SSD1331_begin()
{
    IOExpander::instance().pinMode(PIN_NUM_DISPLAY_RESET, OUTPUT);

    gpio_pad_select_gpio(PIN_NUM_DISPLAY_DC);
    gpio_set_direction((gpio_num_t)PIN_NUM_DISPLAY_DC, GPIO_MODE_OUTPUT);
    
    SSD1331_beginSPI();

    IOExpander::instance().digitalWrite(PIN_NUM_DISPLAY_RESET, 1);
    vTaskDelay(10/portTICK_RATE_MS);
    IOExpander::instance().digitalWrite(PIN_NUM_DISPLAY_RESET, 0);
    vTaskDelay(10/portTICK_RATE_MS);
    IOExpander::instance().digitalWrite(PIN_NUM_DISPLAY_RESET, 1);
    vTaskDelay(10/portTICK_RATE_MS);

    SSD1331_command(DISPLAY_OFF);              //Display Off
    SSD1331_command(SET_CONTRAST_A);           //Set contrast for color A
    SSD1331_command(0xFF);                     //145 0x91
    SSD1331_command(SET_CONTRAST_B);           //Set contrast for color B
    SSD1331_command(0xFF);                     //80 0x50
    SSD1331_command(SET_CONTRAST_C);           //Set contrast for color C
    SSD1331_command(0xFF);                     //125 0x7D
    SSD1331_command(MASTER_CURRENT_CONTROL);   //master current control
    SSD1331_command(0x06);                     //6
    SSD1331_command(SET_PRECHARGE_SPEED_A);    //Set Second Pre-change Speed For ColorA
    SSD1331_command(0x64);                     //100
    SSD1331_command(SET_PRECHARGE_SPEED_B);    //Set Second Pre-change Speed For ColorB
    SSD1331_command(0x78);                     //120
    SSD1331_command(SET_PRECHARGE_SPEED_C);    //Set Second Pre-change Speed For ColorC
    SSD1331_command(0x64);                     //100
    SSD1331_command(SET_REMAP);                //set remap & data format
    SSD1331_command(0x72);                     //0x72              
    SSD1331_command(SET_DISPLAY_START_LINE);   //Set display Start Line
    SSD1331_command(0x0);
    SSD1331_command(SET_DISPLAY_OFFSET);       //Set display offset
    SSD1331_command(0x0);
    SSD1331_command(NORMAL_DISPLAY);           //Set display mode
    SSD1331_command(SET_MULTIPLEX_RATIO);      //Set multiplex ratio
    SSD1331_command(0x3F);                     
    SSD1331_command(SET_MASTER_CONFIGURE);     //Set master configuration
    SSD1331_command(0x8E);                     
    SSD1331_command(POWER_SAVE_MODE);          //Set Power Save Mode
    SSD1331_command(0x00);                     //0x00
    SSD1331_command(PHASE_PERIOD_ADJUSTMENT);  //phase 1 and 2 period adjustment
    SSD1331_command(0x31);                     //0x31
    SSD1331_command(DISPLAY_CLOCK_DIV);        //display clock divider/oscillator frequency
    SSD1331_command(0xF0);
    SSD1331_command(SET_PRECHARGE_VOLTAGE);    //Set Pre-Change Level
    SSD1331_command(0x3A);
    SSD1331_command(SET_V_VOLTAGE);            //Set vcomH
    SSD1331_command(0x3E);                    
    SSD1331_command(DEACTIVE_SCROLLING);       //disable scrolling
    SSD1331_command(NORMAL_BRIGHTNESS_DISPLAY_ON);    //set display on
}

void SSD1331_clear() {
    int i;
    for(i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = 0;
    }
}

void SSD1331_shutdown()
{
    SSD1331_command(DISPLAY_OFF);
}

void SSD1331_draw_point(int x, int y, unsigned short hwColor) {
    if(x >= OLED_WIDTH || y >= OLED_HEIGHT)
    {
        return;
    }
    buffer[x * 2 + y * OLED_WIDTH * 2] = hwColor >> 8;
    buffer[x * 2 + y * OLED_WIDTH * 2 + 1] = hwColor;
}

void SSD1331_rectangle(int x1, int y1, int x2, int y2, unsigned short hwColor, bool filled) {
    
    // Draw border
    SSD1331_line(x1, y1, x1, y2, hwColor);
    SSD1331_line(x1, y1, x2, y1, hwColor);
    SSD1331_line(x1, y2, x2, y2, hwColor);
    SSD1331_line(x2, y1, x2, y2, hwColor);

    // Fill
    if (filled)
    {
        for (uint8_t x = x1+1; x < x2; x++)
        {
            for (uint8_t y = y1+1; y < y2; y++)
            {
                SSD1331_draw_point(x, y, hwColor);
            }
        }
    }
}

void SSD1331_line(int x1, int y1, int x2, int y2, unsigned short hwColor) {

    if (x1 == x2)
    {
        for (uint8_t y = y1; y <= y2; y++)
        {
            SSD1331_draw_point(x1, y, hwColor);
        }
    }

    else if (y1 == y2)
    {
        for (uint8_t x = x1; x <= x2; x++)
        {
            SSD1331_draw_point(x, y1, hwColor);
        }
    }

    else
    {
        float m = (float)(y2-y1) / (float)(x2-x1);
        float b = (float)y1;

        for (uint8_t x = 0; x <= x2-x1; x++)
        {
            uint8_t y = (uint8_t)(m * (float)x + b);
            SSD1331_draw_point(x + x1, y, hwColor);
        }
    }
}

void SSD1331_char1616(unsigned char x, unsigned char y, unsigned char chChar, unsigned short hwColor) {
    unsigned char i, j;
    unsigned char chTemp = 0, y0 = y;

    for (i = 0; i < 32; i ++) {
        chTemp = Font1612[chChar - 0x30][i];
        for (j = 0; j < 8; j ++) {
            if (chTemp & 0x80) {
                SSD1331_draw_point(x, y, hwColor);
            } else {
                SSD1331_draw_point(x, y, 0);
            }
            chTemp <<= 1;
            y++;
            if ((y - y0) == 16) {
                y = y0;
                x++;
                break;
            }
        }
    }
}

void SSD1331_char3216(unsigned char x, unsigned char y, unsigned char chChar, unsigned short hwColor) {
    unsigned char i, j;
    unsigned char chTemp = 0, y0 = y; 

    for (i = 0; i < 64; i++) {
        chTemp = Font3216[chChar - 0x30][i];
        for (j = 0; j < 8; j++) {
            if (chTemp & 0x80) {
                SSD1331_draw_point(x, y, hwColor);
            } else {
                SSD1331_draw_point(x, y, 0);
            }

            chTemp <<= 1;
            y++;
            if ((y - y0) == 32) {
                y = y0;
                x++;
                break;
            }
        }
    }
}

static void SSD1331_char(unsigned char x, unsigned char y, char acsii, char size, char mode, unsigned short hwColor) {
    unsigned char i, j, y0=y;
    char temp;
    unsigned char ch = acsii - ' ';
    for(i = 0;i<size;i++) {
        if(size == 12)
        {
            if(mode)temp=Font1206[ch][i];
            else temp = ~Font1206[ch][i];
        }
        else 
        {            
            if(mode)temp=Font1608[ch][i];
            else temp = ~Font1608[ch][i];
        }
        for(j =0;j<8;j++)
        {
            if(temp & 0x80) SSD1331_draw_point(x, y, hwColor);
            else SSD1331_draw_point(x, y, 0);
            temp <<=1;
            y++;
            if((y-y0)==size)
            {
                y = y0;
                x ++;
                break;
            }
        }
    }
}

void SSD1331_string(unsigned char x, unsigned char y, const char *pString, unsigned char Size, unsigned char Mode, unsigned short hwColor) {
    while (*pString != '\0') {       
        if (x > (OLED_WIDTH - Size / 2)) {
            x = 0;
            y += Size;
            if (y > (OLED_HEIGHT - Size)) {
                y = x = 0;
            }
        }
        
        SSD1331_char(x, y, *pString, Size, Mode, hwColor);
        x += Size / 2;
        pString ++;
    }
}

void SSD1331_mono_bitmap(unsigned char x, unsigned char y, const unsigned char *pBmp, char chWidth, char chHeight, unsigned short hwColor) {
    unsigned char i, j, byteWidth = (chWidth + 7) / 8;
    for(j = 0; j < chHeight; j++) {
        for(i = 0; i <chWidth; i ++) {
            if(*(pBmp + j * byteWidth + i / 8) & (128 >> (i & 7))) {
                SSD1331_draw_point(x + i, y + j, hwColor);
            }
        }
    }        
}

void SSD1331_bitmap24(unsigned char x, unsigned char y, unsigned char *pBmp, char chWidth, char chHeight) {
    unsigned char i, j;
    unsigned short hwColor;
    unsigned int temp;

    for(j = 0; j < chHeight; j++) {
        for(i = 0; i < chWidth; i ++) {
            temp = *(unsigned int*)(pBmp + i * 3 + j * 3 * chWidth);
            hwColor = RGB(((temp >> 16) & 0xFF),
                          ((temp >> 8) & 0xFF),
                           (temp & 0xFF));
            SSD1331_draw_point(x + i, y + chHeight - 1 - j, hwColor);
        }
    }
}

void SSD1331_display() {
    int txLen = 64;
    int remain = sizeof(buffer);
    unsigned char *pBuffer = buffer;
    SSD1331_command(SET_COLUMN_ADDRESS);
    SSD1331_command(0);         //cloumn start address
    SSD1331_command(OLED_WIDTH - 1); //cloumn end address
    SSD1331_command(SET_ROW_ADDRESS);
    SSD1331_command(0);         //page atart address
    SSD1331_command(OLED_HEIGHT - 1); //page end address
    gpio_set_level((gpio_num_t)PIN_NUM_DISPLAY_DC, 1);
    while (remain > txLen)
    {
        SPIWrite(pBuffer, txLen);
        remain -= txLen;
        pBuffer += txLen;
    }
    SPIWrite(pBuffer, remain);
}

void SSD1331_clear_screen(unsigned short hwColor) {
    unsigned short i, j;
    for(i = 0; i < OLED_HEIGHT; i++) {
        for(j = 0; j < OLED_WIDTH; j ++) {
            SSD1331_draw_point(j, i, hwColor);
        }
    }
}

