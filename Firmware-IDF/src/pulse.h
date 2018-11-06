#ifndef _PULSE_H_
#define _PULSE_H_

#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MAX30102.h"


class Pulse
{
    friend void sensetask(void *pvParameters);
    
public:
    static Pulse* instance();

    void getLedVal(uint32_t * pun_ir_led, uint32_t * pun_red_led,uint8_t *k);

    private:
    static Pulse* _instance;
    Pulse();
    virtual ~Pulse();

    MAX30102 _max30102;
    TaskHandle_t _pulseTaskHandle;
};

#endif