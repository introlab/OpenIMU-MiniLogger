#include "ADS1015.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

namespace
{
    void IRAM_ATTR ads1015_gpio_isr_handler(void* arg)
    {
        //DO something
        ADS1015 *adc = (ADS1015*) (arg);
        assert(adc);
        xSemaphoreGiveFromISR(adc->getSemaphore(), NULL);
    }

}


ADS1015::ADS1015(i2c_port_t port, uint8_t address)
: _port(port), _address(address)
{
    setup();
}

void ADS1015::setup()
{
    gpio_config_t io_conf;
    //interrupt on falling edge
    io_conf.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_NEGEDGE;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO33
    io_conf.pin_bit_mask =  (1ULL << PIN_NUM_ADC_READY);
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t) 0;
    //enable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t) 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);


    /*
    The ALERT/RDY pin can also be configured as a conversion ready pin. Set the most-significant bit of the
    Hi_thresh register to 1 and the most-significant bit of Lo_thresh register to 0 to enable the pin as a conversion
    ready pin.
    */

    writeRegister(_address, ADS1015_REG_POINTER_HITHRESH, 0x8000);
    writeRegister(_address, ADS1015_REG_POINTER_LOWTHRESH, 0x7FFF);

    _semaphore = xSemaphoreCreateCounting(1,0);
    
    gpio_isr_handler_add((gpio_num_t)PIN_NUM_ADC_READY, ads1015_gpio_isr_handler, this);
}

uint16_t ADS1015::readADC_SingleEnded(uint8_t channel)
{

    // Start with default values
    uint16_t config = ADS1015_REG_CONFIG_CQUE_1CONV    | // Enable comparator, 1 conv
                    ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_DR_128SPS   | // 1600 samples per second (default)
                    ADS1015_REG_CONFIG_MODE_SINGLE;    // Single-shot mode (default)
                    

    // Set PGA/voltage range
    config |= (uint16_t) GAIN_ONE; //GAIN = 1;

    // Set single-ended input channel
    switch (channel)
    {
        case (0):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
        break;
        case (1):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
        break;
        case (2):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
        break;
        case (3):
        config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
        break;
    }

    // Set 'start single-conversion' bit
    config |= ADS1015_REG_CONFIG_OS_SINGLE;

    //printf("Pin state before: %i\n", gpio_get_level((gpio_num_t)PIN_NUM_ADC_READY));
    // Write config register to the ADC
    writeRegister(_address, ADS1015_REG_POINTER_CONFIG, config);

    //Will wait for fallling edge
    // Wait for the conversion to complete, read ready signal
    if (xSemaphoreTake(_semaphore, 1000 / portTICK_RATE_MS) != pdTRUE)
        printf("ERROR: could not wait for adc semaphore\n");

    // Read the conversion results
    // Shift 12-bit results right 4 bits for the ADS1015
    return readRegister(_address, ADS1015_REG_POINTER_CONVERT) >> 4;
}


uint16_t ADS1015::readRegister(uint8_t i2cAddress, uint8_t reg)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2cAddress << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, (reg), ACK_CHECK_EN);
    //i2c_master_stop(cmd);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2cAddress << 1 ) | I2C_MASTER_READ, ACK_CHECK_EN);

    //data to read
    uint8_t data[2];

    i2c_master_read_byte(cmd, &data[0], (i2c_ack_type_t) ACK_VAL);
    i2c_master_read_byte(cmd, &data[1], (i2c_ack_type_t) NACK_VAL);
    i2c_master_stop(cmd);

    /*
        I2C master send queued commands. This function will trigger sending all queued commands. 
        The task will be blocked until all the commands have been sent out. 
        The I2C APIs are not thread-safe, if you want to use one I2C port in different tasks, 
        you need to take care of the multi-thread issue.
    */

    esp_err_t ret = I2CBus::i2c_master_cmd_begin(cmd, 1000 / portTICK_RATE_MS);
    if (ret != ESP_OK)
        printf("Error: ADS1015::readRegister ret: %i %2.2x%2.2x\n", ret, data[0], data[1]);

    i2c_cmd_link_delete(cmd);

    return (data[0] << 8) | (data[1] & 0xFF);
}

esp_err_t ADS1015::writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value)
{
    //Build commands for I2C
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( i2cAddress << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, (value >> 8), ACK_CHECK_EN);
    i2c_master_write_byte(cmd, (value & 0xFF), ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = I2CBus::i2c_master_cmd_begin(cmd, 1000 / portTICK_RATE_MS);
    if (ret != ESP_OK)
        printf("Error: ADC1015::writeRegister ret: %i\n", ret);

    i2c_cmd_link_delete(cmd);
    return ret;
}



