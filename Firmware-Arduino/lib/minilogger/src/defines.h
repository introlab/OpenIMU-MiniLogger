#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <Arduino.h>
#include <stdint.h>
#include <string>

/**
 * Put global definitions here
 * 
 **/

//MAX30102 definitions
#define MAX30102_FIFO_SIZE 32 
#define PULSE_BUFFER_LENGHT 100


typedef struct {
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    float magX;
    float magY;
    float magZ;
} imuData_t;
typedef imuData_t* imuDataPtr_t;

typedef struct {
    float voltage;
    float current;
} powerData_t;
typedef powerData_t* powerDataPtr_t;


typedef struct {
    float temperature;
    float pressure;
} baroData_t;
typedef baroData_t* baroDataPtr_t;


typedef struct {
    bool fix;
    float latitude;
    float longitude;
    float altitude;
} gpsData_t;
typedef gpsData_t* gpsDataPtr_t;


typedef struct {
    uint32_t ir_led_data[PULSE_BUFFER_LENGHT];
    uint32_t red_led_data[PULSE_BUFFER_LENGHT];
    float spo2;
    int heartrate;
} pulseData_t;
typedef pulseData_t* pulseDataPtr_t;

typedef struct {
    int IMUSampleRate;
    int IMUGyroRange;
    int IMUAcellRange;
}  IMUconfig_Sd;
//typedef IMUconfig_Sd* IMUconfigPtr_Sd;

typedef struct {
    std::string deviceName;
    std::string openTeraServerName;
    int openTeraServerPort;
    std::string openTeraToken;
} OpenTeraConfig_Sd;

typedef struct {
    std::string ssid;
    std::string password;
} WiFiConfig_Sd;

#ifndef ON
#define ON (1)
#endif

#ifndef OFF
#define OFF (0)
#endif

//I/O Expander pins (id is +1 of index)
#define EXT_PIN00_DISPLAY_RESET 1
#define EXT_PIN01_LED 2
#define EXT_PIN02_BUTTON0 3
#define EXT_PIN03_SD_N_ENABLED 4
#define EXT_PIN04_SD_N_CD 5
#define EXT_PIN05_SD_SEL 6
#define EXT_PIN06_BUTTON1 7
#define EXT_PIN07_VUSB 8
#define EXT_PIN08_BUTTON2 9
#define EXT_PIN09_BUTTON3 10
#define EXT_PIN10_CHARGING 11
#define EXT_PIN11_UNUSED 12
#define EXT_PIN12_P5_PIN7 13
#define EXT_PIN13_BATT_READ_EN 14
#define EXT_PIN14_EXTERNAL_POWER_EN 15
#define EXT_PIN15_MOTOR_VIBRATE 16

#define IO_EXPANDER_PIN_NUM_CS   5

//SPI definitions
#define PIN_NUM_MISO 39
#define PIN_NUM_MOSI 18
#define PIN_NUM_CLK  19

//I2C definitions
#define PIN_NUM_SDA 23
#define PIN_NUM_SCL 25

//GPS definitions
#define PIN_NUM_UART_TX 26 
#define PIN_NUM_UART_RX 35

//I2C definitions EXT
#define PIN_NUM_EXTSDA 17
#define PIN_NUM_EXTSCL 16

//ADC definitions
#define PIN_NUM_ADC_READY 33
#define ADC_VOLTAGE_CHANNEL 0
#define ADC_CURRENT_CHANNEL 1
#define ADC_THERM_CHANNEL 2

//Display definitions
#define PIN_NUM_DISPLAY_RESET EXT_PIN00_DISPLAY_RESET
#define PIN_NUM_DISPLAY_CS 21
#define PIN_NUM_DISPLAY_DC 22

// System definitions
#define PIN_NUM_ENABLE_PROGRAMMING 27
#define SCREEN_DIM_TIMER 5000
#define SCREEN_SLEEP_TIMER 15000

#endif