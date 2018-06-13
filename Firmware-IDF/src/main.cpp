#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

// REGISTERS ARE DEFINED HERE SO THAT THEY MAY BE USED IN THE MAIN PROGRAM

#define    IODIRA    (0x00)      // MCP23x17 I/O Direction Register
#define    IODIRB    (0x01)      // 1 = Input (default), 0 = Output

#define    IPOLA     (0x02)      // MCP23x17 Input Polarity Register
#define    IPOLB     (0x03)      // 0 = Normal (default)(low reads as 0), 1 = Inverted (low reads as 1)

#define    GPINTENA  (0x04)      // MCP23x17 Interrupt on Change Pin Assignements
#define    GPINTENB  (0x05)      // 0 = No Interrupt on Change (default), 1 = Interrupt on Change

#define    DEFVALA   (0x06)      // MCP23x17 Default Compare Register for Interrupt on Change
#define    DEFVALB   (0x07)      // Opposite of what is here will trigger an interrupt (default = 0)

#define    INTCONA   (0x08)      // MCP23x17 Interrupt on Change Control Register
#define    INTCONB   (0x09)      // 1 = pin is compared to DEFVAL, 0 = pin is compared to previous state (default)

#define    IOCON     (0x0A)      // MCP23x17 Configuration Register
//                   (0x0B)      //     Also Configuration Register

#define    GPPUA     (0x0C)      // MCP23x17 Weak Pull-Up Resistor Register
#define    GPPUB     (0x0D)      // INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up 

#define    INTFA     (0x0E)      // MCP23x17 Interrupt Flag Register
#define    INTFB     (0x0F)      // READ ONLY: 1 = This Pin Triggered the Interrupt

#define    INTCAPA   (0x10)      // MCP23x17 Interrupt Captured Value for Port Register
#define    INTCAPB   (0x11)      // READ ONLY: State of the Pin at the Time the Interrupt Occurred

#define    GPIOA     (0x12)      // MCP23x17 GPIO Port Register
#define    GPIOB     (0x13)      // Value on the Port - Writing Sets Bits in the Output Latch

#define    OLATA     (0x14)      // MCP23x17 Output Latch Register
#define    OLATB     (0x15)      // 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value!

#define    OPCODEW       (0b01000000)  // Opcode for MCP23S17 with LSB (bit0) set to write (0), address OR'd in later, bits 1-3
#define    OPCODER       (0b01000001)  // Opcode for MCP23S17 with LSB (bit0) set to read (1), address OR'd in later, bits 1-3
#define    ADDR_ENABLE   (0b00001000)  // Configuration register for MCP23S17, the only thing we change is enabling hardware addressing


#define PIN_NUM_MISO 39
#define PIN_NUM_MOSI 18
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   5


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


#define OUTPUT 0
#define INPUT 1
#define HIGH 1
#define LOW 0
#ifndef ON
#define    ON            (1)
#endif

#ifndef OFF
#define    OFF           (0)
#endif

class IOExpander
{
    public:

    IOExpander(int addr, gpio_num_t cs_pin, spi_device_handle_t dev)
        : m_address(addr), m_cs_pin(cs_pin), m_spiDev(dev)
    {
        _modeCache   = 0xFFFF;                // Default I/O mode is all input, 0xFFFF
        _outputCache = 0x0000;                // Default output state is all off, 0x0000
        _pullupCache = 0x0000;                // Default pull-up state is all off, 0x0000
        _invertCache = 0x0000;                // Default input inversion state is not inverted, 0x0000

        printf("Dev handle: %p\n",  m_spiDev);

        //ADDR 0, CS=PIN5
        setup();
    }

    void setup()
    {
        //SETUP GPIO
        gpio_config_t io_conf;
        //disable interrupt
        io_conf.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_DISABLE;
        //set as output mode
        io_conf.mode = GPIO_MODE_OUTPUT;
        //bit mask of the pins that you want to set,e.g.GPIO18/19
        io_conf.pin_bit_mask =  (1ULL << m_cs_pin);
        //disable pull-down mode
        io_conf.pull_down_en = (gpio_pulldown_t) 0;
        //disable pull-up mode
        io_conf.pull_up_en = (gpio_pullup_t) 0;
        //configure GPIO with the given settings
        gpio_config(&io_conf);


        //Enable addressing
        byteWrite(IOCON, ADDR_ENABLE);
    }

    void pullupMode(uint8_t pin, uint8_t mode) {
        if (pin < 1 || pin > 16) return;
        if (mode == ON) {
            _pullupCache |= 1 << (pin - 1);
        } else {
            _pullupCache &= ~(1 << (pin -1));
        }
        wordWrite(GPPUA, _pullupCache);
    }

    void pinMode(uint8_t pin, uint8_t mode) 
    {  
        // Accept the pin # and I/O mode
        if (pin < 1 || pin > 16) return;               // If the pin value is not valid (1-16) return, do nothing and return
        if (mode == INPUT) {                          // Determine the mode before changing the bit state in the mode cache
            _modeCache |= 1 << (pin - 1);               // Since input = "HIGH", OR in a 1 in the appropriate place
        } else {
            _modeCache &= ~(1 << (pin - 1));            // If not, the mode must be output, so and in a 0 in the appropriate place
        }
        wordWrite(IODIRA, _modeCache);                // Call the generic word writer with start register and the mode cache
    }

    uint8_t digitalRead(uint8_t pin)
    {
        //TODO
        return 0;
    }

    void digitalWrite(uint8_t pin, uint8_t value)
    {
        if (pin < 1 || pin > 16) return;
        if (value) {
            _outputCache |= 1 << (pin - 1);
        } else {
            _outputCache &= ~(1 << (pin - 1));
        }
        wordWrite(GPIOA, _outputCache);
    }

    protected:
   
    void byteWrite(uint8_t reg, uint8_t value)
    {
        
        /*
            SPI.transfer(OPCODEW | (_address << 1));             // Send the MCP23S17 opcode, chip address, and write bit
            SPI.transfer(reg);                                   // Send the register we want to write
            SPI.transfer(value);   
        */


        //Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
        //function is finished because the SPI driver needs access to it even while we're already calculating the next line.
        spi_transaction_t trans;
        memset(&trans, 0, sizeof(spi_transaction_t));
        trans.flags=0;
        trans.addr = OPCODEW | (m_address << 1);//Address
        uint8_t data[2];
        trans.tx_buffer = data;
        data[0] = reg;
        data[1] = value;
        trans.length = 2  * 8; // in bits
        esp_err_t ret = ESP_OK;

       
        //Queue and wait for result, not thread safe
        ret = spi_device_transmit(m_spiDev, &trans);
        

        printf("SPI byteWrite returned: %i \n", ret);

    }

    void wordWrite(uint8_t reg, unsigned int word)
    {
        /*
          SPI.transfer(OPCODEW | (_address << 1));             // Send the MCP23S17 opcode, chip address, and write bit
            SPI.transfer(reg);                                   // Send the register we want to write
            SPI.transfer((uint8_t) (word));                      // Send the low byte (register address pointer will auto-increment after write)
            SPI.transfer((uint8_t) (word >> 8)); 
        
        */
        spi_transaction_t trans;
        memset(&trans, 0, sizeof(spi_transaction_t));
        trans.flags=0;
        trans.addr = OPCODEW | (m_address << 1);//Address
        uint8_t data[10];
        trans.tx_buffer = data;
        data[0] = reg;
        data[1] = (uint8_t) word;
        data[2] = (uint8_t) (word >> 8);
        trans.length = 3 * 8; //in bits
        
        esp_err_t ret = ESP_OK;

        //Queue and wait for result, not thread safe
        ret = spi_device_transmit(m_spiDev, &trans);

        printf("SPI wordWrite returned: %i \n", ret);

    }

    int m_address;
    gpio_num_t m_cs_pin;
    spi_device_handle_t m_spiDev;
    unsigned int _modeCache;// Caches the mode (input/output) configuration of I/O pins
    unsigned int _pullupCache;// Caches the internal pull-up configuration of input pins (values persist across mode changes)
    unsigned int _invertCache;// Caches the input pin inversion selection (values persist across mode changes)
    unsigned int _outputCache;// Caches the output pin state of pins

};


   void cs_active(spi_transaction_t* trans)
    {
        //printf("cs_active\n");
        gpio_set_level((gpio_num_t)PIN_NUM_CS, 0);
    }

    void cs_inactive(spi_transaction_t* trans)
    {
        //printf("cs_inactive\n");
        gpio_set_level((gpio_num_t)PIN_NUM_CS, 1);
    }


//app_main should have a "C" signature
extern "C" 
{
 

    void app_main()
    {
        esp_err_t ret;

        //SPI Device handle
        spi_device_handle_t dev;

        //SPI bus configuration
        spi_bus_config_t buscfg;
        buscfg.miso_io_num = PIN_NUM_MISO;
        buscfg.mosi_io_num = PIN_NUM_MOSI;
        buscfg.sclk_io_num = PIN_NUM_CLK;
        buscfg.quadwp_io_num = -1;
        buscfg.quadhd_io_num = -1;
        buscfg.max_transfer_sz = 1000;
  
        //SPI interface configuration
        spi_device_interface_config_t dev_config;
        memset(&dev_config, 0, sizeof(spi_device_interface_config_t));
        dev_config.command_bits = 0;
        dev_config.address_bits = 8;
        dev_config.dummy_bits = 0;
        dev_config.mode = 0;
        dev_config.duty_cycle_pos = 128;  // default 128 = 50%/50% duty
        dev_config.cs_ena_pretrans = 0;  // 0 not used
        dev_config.cs_ena_posttrans = 0;  // 0 not used
        dev_config.clock_speed_hz = 10000000;
        dev_config.spics_io_num = -1;
        dev_config.flags = 0;  // 0 not used
        dev_config.queue_size = 1;
        dev_config.pre_cb = cs_active;
        dev_config.post_cb = cs_inactive;
        
        
    
        vTaskDelay(500 / portTICK_RATE_MS);
 
        //Initialize the SPI bus data structure, no DMA for now
        ret = spi_bus_initialize(HSPI_HOST, &buscfg, 0);
        printf("SPI BUS RET : %i\n", ret);
        //assert(ret == ESP_OK);

        vTaskDelay(500 / portTICK_RATE_MS);

        //Setup SPI Dev
        ret = spi_bus_add_device(HSPI_HOST, &dev_config, &dev);
        printf("SPI BUS ADD DEVICE RET: %i handle: %p\n", ret, dev);
        //assert(ret == ESP_OK);

        //nvs_flash_init();
        //xTaskCreate(&blinky, "blinky", 512,NULL,5,NULL );

        IOExpander ioExpander(0, (gpio_num_t) 5, dev);

        //ALIVE -->HIGH, power will stay on
        ioExpander.pinMode(EXT_PIN12_KEEP_ALIVE, OUTPUT);
        ioExpander.digitalWrite(EXT_PIN12_KEEP_ALIVE, HIGH);

        //LED
        ioExpander.pinMode(EXT_PIN01_LED, OUTPUT);
        ioExpander.digitalWrite(EXT_PIN01_LED, HIGH);


        //Do better...
        while(1)
        {
            printf("Hello World!\n");
            ioExpander.digitalWrite(EXT_PIN01_LED, HIGH); 
            vTaskDelay(500 / portTICK_RATE_MS);
            ioExpander.digitalWrite(EXT_PIN01_LED, LOW);
            vTaskDelay(500 / portTICK_RATE_MS);
        }
    }
}