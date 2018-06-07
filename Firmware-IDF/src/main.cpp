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


#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 18
#define PIN_NUM_CLK  39
#define PIN_NUM_CS   5


class IOExpander
{
    public:

    IOExpander(int addr, gpio_num_t cs_pin, spi_device_handle_t* dev)
        : m_address(addr), m_cs_pin(cs_pin), m_spiDev(dev)
    {
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
    }

    void pullupMode(uint8_t pinNo, uint8_t mode)
    {

    }

    uint8_t digitalRead(uint8_t pinNo)
    {
        return 0;
    }

    void digitalWrite(uint8_t pinNo, uint8_t value)
    {

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
        trans.flags=SPI_TRANS_USE_TXDATA;
        trans.tx_data[0]= OPCODEW | (m_address << 1);//Address
        trans.tx_data[1] = reg;
        trans.tx_data[2] = value;

        gpio_set_level(m_cs_pin, 0);
        

        gpio_set_level(m_cs_pin, 1);

    }

    void wordWrite(uint8_t reg, unsigned int word)
    {
        /*
          SPI.transfer(OPCODEW | (_address << 1));             // Send the MCP23S17 opcode, chip address, and write bit
            SPI.transfer(reg);                                   // Send the register we want to write
            SPI.transfer((uint8_t) (word));                      // Send the low byte (register address pointer will auto-increment after write)
            SPI.transfer((uint8_t) (word >> 8)); 
        
        */

        gpio_set_level(m_cs_pin, 0);


        gpio_set_level(m_cs_pin, 1);

    }

    int m_address;
    gpio_num_t m_cs_pin;
    spi_device_handle_t *m_spiDev;

};

void blinky(void *pvParameter)
{
    while(1)
    {
        printf("Hello world!\n");
        vTaskDelay(500 / portTICK_RATE_MS);
        

        vTaskDelay(500 / portTICK_RATE_MS);
    }
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
        spi_device_interface_config_t devcfg;
        
        devcfg.clock_speed_hz = 10000000; //10MHz
        devcfg.mode = 0;
        devcfg.spics_io_num = -1; //Handled by code
        devcfg.queue_size = 1; //One transaction at a time
 
        //Initialize the SPI bus data structure
        ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
        assert(ret == ESP_OK);

        //Setup SPI Dev
        ret = spi_bus_add_device(HSPI_HOST, &devcfg, &dev);
        assert(ret == ESP_OK);

        //nvs_flash_init();
        xTaskCreate(&blinky, "blinky", 512,NULL,5,NULL );

        //Do better...
        while(1)
        {
            printf("Hello World!\n");
            vTaskDelay(500 / portTICK_RATE_MS);
        }
    }
}