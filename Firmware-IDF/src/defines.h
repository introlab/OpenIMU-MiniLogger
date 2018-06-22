#ifndef _DEFINES_H_
#define _DEFINES_H_

/**
 * Put global definitions here
 * 
 **/

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

#ifndef OUTPUT
#define OUTPUT (0)
#endif

#ifndef INPUT
#define INPUT (1)
#endif

#ifndef HIGH
#define HIGH (1)
#endif

#ifndef LOW
#define LOW (0)
#endif

#ifndef ON
#define ON (1)
#endif

#ifndef OFF
#define OFF (0)
#endif


//I/O Expander pins (id is +1 of index)
#define EXT_PIN00_DISPLAY_RESET 1
#define EXT_PIN01_LED 2
#define EXT_PIN02_UNUSED 3
#define EXT_PIN03_SD_N_ENABLED 4
#define EXT_PIN04_SD_N_CD 5
#define EXT_PIN05_SD_SEL 6
#define EXT_PIN06_BUTTON1 7
#define EXT_PIN07_VUSB 8
#define EXT_PIN08_BUTTON2 9
#define EXT_PIN09_BUTTON3 10
#define EXT_PIN10_CHARGING 11
#define EXT_PIN11_BUTTON0 12
#define EXT_PIN12_KEEP_ALIVE 13
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


//ADC definitions
#define PIN_NUM_ADC_READY 33
#define ADC_VOLTAGE_CHANNEL 0
#define ADC_CURRENT_CHANNEL 1
#define ADC_THERM_CHANNEL 2

#endif