#ifndef MAX30102_H_
#define MAX30102_H_

#include "defines.h"
#include <stdio.h>
#include "i2cbusext.h"


/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
    #define MAX30102_ADDR                           (0xAE) 
/*=========================================================================*/

/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/
    #define REG_INTR_STATUS_1                       (0x00)
    #define REG_INTR_STATUS_2                       (0x01)
    #define REG_INTR_ENABLE_1                       (0x02)
    #define REG_INTR_ENABLE_2                       (0x03)
    #define REG_FIFO_WR_PTR                         (0x04)
    #define REG_OVF_COUNTER                         (0x05)
    #define REG_FIFO_RD_PTR                         (0x06)
    #define REG_FIFO_DATA                           (0x07)
    #define REG_FIFO_CONFIG                         (0x08)
    #define REG_MODE_CONFIG                         (0x09)
    #define REG_SPO2_CONFIG                         (0x0A)
    #define REG_LED1_PA                             (0x0C)
    #define REG_LED2_PA                             (0x0D)
    #define REG_PILOT_PA                            (0x10)
    #define REG_MULTI_LED_CTRL1                     (0x11)
    #define REG_MULTI_LED_CTRL2                     (0x12)
    #define REG_TEMP_INTR                           (0x1F)
    #define REG_TEMP_FRAC                           (0x20)
    #define REG_TEMP_CONFIG                         (0x21)
    #define REG_PROX_INT_THRESH                     (0x30)
    #define REG_REV_ID                              (0xFE)
    #define REG_PART_ID                             (0xFF)
/*=========================================================================*/

class MAX30102
{
    public:
    MAX30102(i2c_port_t port, uint8_t address=MAX30102_ADDR);

    esp_err_t readFIFO(uint32_t * pun_red_led, uint32_t * pun_ir_led,uint8_t *k);
    esp_err_t init_config();

    private:
    
    i2c_port_t _port;
    uint8_t _address;

    
   
    esp_err_t readRegisters(uint8_t reg, uint8_t count, uint8_t* dest);
    esp_err_t writeRegister(uint8_t reg, uint8_t data);

};


#endif /* MAX30102_H_ */