#include "adc.h"
#include "defines.h"
/*!

    Inspired by:
    
    @file     Adafruit_ADS1015.cpp
    @author   K.Townsend (Adafruit Industries)
    @license  BSD (see license.txt)

    Driver for the ADS1015/ADS1115 ADC

    This is a library for the Adafruit MPL115A2 breakout
    ----> https://www.adafruit.com/products/???

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0 - First release
*/


/*=========================================================================
    CONVERSION DELAY (in mS)
    -----------------------------------------------------------------------*/
    #define ADS1015_CONVERSIONDELAY         (1)
/*=========================================================================*/

/*=========================================================================
    POINTER REGISTER
    -----------------------------------------------------------------------*/
    #define ADS1015_REG_POINTER_MASK        (0x03)
    #define ADS1015_REG_POINTER_CONVERT     (0x00)
    #define ADS1015_REG_POINTER_CONFIG      (0x01)
    #define ADS1015_REG_POINTER_LOWTHRESH   (0x02)
    #define ADS1015_REG_POINTER_HITHRESH    (0x03)
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTER
    -----------------------------------------------------------------------*/
    #define ADS1015_REG_CONFIG_OS_MASK      (0x8000)
    #define ADS1015_REG_CONFIG_OS_SINGLE    (0x8000)  // Write: Set to start a single-conversion
    #define ADS1015_REG_CONFIG_OS_BUSY      (0x0000)  // Read: Bit = 0 when conversion is in progress
    #define ADS1015_REG_CONFIG_OS_NOTBUSY   (0x8000)  // Read: Bit = 1 when device is not performing a conversion

    #define ADS1015_REG_CONFIG_MUX_MASK     (0x7000)
    #define ADS1015_REG_CONFIG_MUX_DIFF_0_1 (0x0000)  // Differential P = AIN0, N = AIN1 (default)
    #define ADS1015_REG_CONFIG_MUX_DIFF_0_3 (0x1000)  // Differential P = AIN0, N = AIN3
    #define ADS1015_REG_CONFIG_MUX_DIFF_1_3 (0x2000)  // Differential P = AIN1, N = AIN3
    #define ADS1015_REG_CONFIG_MUX_DIFF_2_3 (0x3000)  // Differential P = AIN2, N = AIN3
    #define ADS1015_REG_CONFIG_MUX_SINGLE_0 (0x4000)  // Single-ended AIN0
    #define ADS1015_REG_CONFIG_MUX_SINGLE_1 (0x5000)  // Single-ended AIN1
    #define ADS1015_REG_CONFIG_MUX_SINGLE_2 (0x6000)  // Single-ended AIN2
    #define ADS1015_REG_CONFIG_MUX_SINGLE_3 (0x7000)  // Single-ended AIN3

    #define ADS1015_REG_CONFIG_PGA_MASK     (0x0E00)
    #define ADS1015_REG_CONFIG_PGA_6_144V   (0x0000)  // +/-6.144V range = Gain 2/3
    #define ADS1015_REG_CONFIG_PGA_4_096V   (0x0200)  // +/-4.096V range = Gain 1
    #define ADS1015_REG_CONFIG_PGA_2_048V   (0x0400)  // +/-2.048V range = Gain 2 (default)
    #define ADS1015_REG_CONFIG_PGA_1_024V   (0x0600)  // +/-1.024V range = Gain 4
    #define ADS1015_REG_CONFIG_PGA_0_512V   (0x0800)  // +/-0.512V range = Gain 8
    #define ADS1015_REG_CONFIG_PGA_0_256V   (0x0A00)  // +/-0.256V range = Gain 16

    #define ADS1015_REG_CONFIG_MODE_MASK    (0x0100)
    #define ADS1015_REG_CONFIG_MODE_CONTIN  (0x0000)  // Continuous conversion mode
    #define ADS1015_REG_CONFIG_MODE_SINGLE  (0x0100)  // Power-down single-shot mode (default)

    #define ADS1015_REG_CONFIG_DR_MASK      (0x00E0)  
    #define ADS1015_REG_CONFIG_DR_128SPS    (0x0000)  // 128 samples per second
    #define ADS1015_REG_CONFIG_DR_250SPS    (0x0020)  // 250 samples per second
    #define ADS1015_REG_CONFIG_DR_490SPS    (0x0040)  // 490 samples per second
    #define ADS1015_REG_CONFIG_DR_920SPS    (0x0060)  // 920 samples per second
    #define ADS1015_REG_CONFIG_DR_1600SPS   (0x0080)  // 1600 samples per second (default)
    #define ADS1015_REG_CONFIG_DR_2400SPS   (0x00A0)  // 2400 samples per second
    #define ADS1015_REG_CONFIG_DR_3300SPS   (0x00C0)  // 3300 samples per second

    #define ADS1015_REG_CONFIG_CMODE_MASK   (0x0010)
    #define ADS1015_REG_CONFIG_CMODE_TRAD   (0x0000)  // Traditional comparator with hysteresis (default)
    #define ADS1015_REG_CONFIG_CMODE_WINDOW (0x0010)  // Window comparator

    #define ADS1015_REG_CONFIG_CPOL_MASK    (0x0008)
    #define ADS1015_REG_CONFIG_CPOL_ACTVLOW (0x0000)  // ALERT/RDY pin is low when active (default)
    #define ADS1015_REG_CONFIG_CPOL_ACTVHI  (0x0008)  // ALERT/RDY pin is high when active

    #define ADS1015_REG_CONFIG_CLAT_MASK    (0x0004)  // Determines if ALERT/RDY pin latches once asserted
    #define ADS1015_REG_CONFIG_CLAT_NONLAT  (0x0000)  // Non-latching comparator (default)
    #define ADS1015_REG_CONFIG_CLAT_LATCH   (0x0004)  // Latching comparator

    #define ADS1015_REG_CONFIG_CQUE_MASK    (0x0003)
    #define ADS1015_REG_CONFIG_CQUE_1CONV   (0x0000)  // Assert ALERT/RDY after one conversions
    #define ADS1015_REG_CONFIG_CQUE_2CONV   (0x0001)  // Assert ALERT/RDY after two conversions
    #define ADS1015_REG_CONFIG_CQUE_4CONV   (0x0002)  // Assert ALERT/RDY after four conversions
    #define ADS1015_REG_CONFIG_CQUE_NONE    (0x0003)  // Disable the comparator and put ALERT/RDY in high state (default)
/*=========================================================================*/

typedef enum
{
  GAIN_TWOTHIRDS    = ADS1015_REG_CONFIG_PGA_6_144V,
  GAIN_ONE          = ADS1015_REG_CONFIG_PGA_4_096V,
  GAIN_TWO          = ADS1015_REG_CONFIG_PGA_2_048V,
  GAIN_FOUR         = ADS1015_REG_CONFIG_PGA_1_024V,
  GAIN_EIGHT        = ADS1015_REG_CONFIG_PGA_0_512V,
  GAIN_SIXTEEN      = ADS1015_REG_CONFIG_PGA_0_256V
} adsGain_t;

#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */


namespace
{
    void IRAM_ATTR adc_gpio_isr_handler(void* arg)
    {
       //DO something
       ADC *adc = (ADC*) (arg);

       xSemaphoreGiveFromISR(adc->getSemaphore(), NULL);
    }

}

ADC::ADC(i2c_port_t port, uint8_t address)
    : _port(port), _address(address)
{
    setup();
}

void ADC::setup()
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
    
    gpio_isr_handler_add((gpio_num_t)PIN_NUM_ADC_READY, adc_gpio_isr_handler, this);

}


uint16_t ADC::readADC_SingleEnded(uint8_t channel)
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
    xSemaphoreTake(_semaphore, portMAX_DELAY);

    // Read the conversion results
    // Shift 12-bit results right 4 bits for the ADS1015
    return readRegister(_address, ADS1015_REG_POINTER_CONVERT) >> 4;

}


/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
esp_err_t ADC::writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value) 
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
        printf("Error: ADC::writeRegister ret: %i\n", ret);

    i2c_cmd_link_delete(cmd);
    return ret;
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
uint16_t ADC::readRegister(uint8_t i2cAddress, uint8_t reg) 
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
    i2c_master_read_byte(cmd, &data[1], (i2c_ack_type_t) ACK_VAL);
    i2c_master_stop(cmd);

    /*
        I2C master send queued commands. This function will trigger sending all queued commands. 
        The task will be blocked until all the commands have been sent out. 
        The I2C APIs are not thread-safe, if you want to use one I2C port in different tasks, 
        you need to take care of the multi-thread issue.
    */

    esp_err_t ret = I2CBus::i2c_master_cmd_begin(cmd, 1000 / portTICK_RATE_MS);
    if (ret != ESP_OK)
        printf("Error: ADC::readRegister ret: %i %2.2x%2.2x\n", ret, data[0], data[1]);

    i2c_cmd_link_delete(cmd);

    return (data[0] << 8) | (data[1] & 0xFF);
}

float ADC::read_voltage()
{
    uint16_t value = readADC_SingleEnded(ADC_VOLTAGE_CHANNEL);
    //printf("VOLTAGE HEX: %4.4x, %i\n", value, value);
    return 5.0 * 0.002 * (float) value;
}

float ADC::read_current()
{
    uint16_t value = readADC_SingleEnded(ADC_CURRENT_CHANNEL);
    //printf("CURRENT HEX: %4.4x, %i\n", value, value);
    return ((0.002 * (float) value) - (3.1/2.0)) / 5.0;
}