#ifndef _DEFINES_H_
#define _DEFINES_H_

//#include "MCP23S17.h"
#include "ioexpander.h"

/**
 * GLOBAL DEFINITIONS
 *
*/

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

//external GPIO driver
extern IOExpander ioExpander;


#endif
