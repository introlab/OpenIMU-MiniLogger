#include "ioexpander.h"
#include <string.h>

// REGISTERS 
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

//Anonymous namespace
namespace
{

    void cs_active(spi_transaction_t* trans)
    {
        gpio_set_level((gpio_num_t)IO_EXPANDER_PIN_NUM_CS, 0);
    }

    void cs_inactive(spi_transaction_t* trans)
    {
        gpio_set_level((gpio_num_t)IO_EXPANDER_PIN_NUM_CS, 1);
    }
}

IOExpander* IOExpander::instance()
{
    static IOExpander expander(0, (gpio_num_t) 5, HSPI_HOST);
    return &expander;
}

IOExpander::IOExpander(int addr, gpio_num_t cs_pin, spi_host_device_t host_device)
    : _address(addr), _cs_pin(cs_pin), _host_device(host_device)
{
    _modeCache   = 0xFFFF;                // Default I/O mode is all input, 0xFFFF
    _outputCache = 0x0000;                // Default output state is all off, 0x0000
    _pullupCache = 0x0000;                // Default pull-up state is all off, 0x0000
    _invertCache = 0x0000;                // Default input inversion state is not inverted, 0x0000

    _mutex = xSemaphoreCreateMutex();
    assert(_mutex != NULL);
    

    setup();
}


void IOExpander::setup()
{
    //SETUP GPIO
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask =  (1ULL << _cs_pin);
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t) 0;
    //disable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t) 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //SPI device interface configuration
    memset(&_config, 0, sizeof(spi_device_interface_config_t));
    _config.command_bits = 0;
    _config.address_bits = 8;
    _config.dummy_bits = 0;
    _config.mode = 0;
    _config.duty_cycle_pos = 128;  // default 128 = 50%/50% duty
    _config.cs_ena_pretrans = 0;  // 0 not used
    _config.cs_ena_posttrans = 0;  // 0 not used
    _config.clock_speed_hz = 10000000;
    _config.spics_io_num = -1;
    _config.flags = 0;  // 0 not used
    _config.queue_size = 1;
    _config.pre_cb = cs_active;
    _config.post_cb = cs_inactive;

    esp_err_t ret = spi_bus_add_device(HSPI_HOST, &_config, &_handle);
    printf("SPI BUS ADD DEVICE RET: %i handle: %p\n", ret, _handle);
    assert(ret == ESP_OK);

    //Enable addressing
    ret = byteWrite(IOCON, ADDR_ENABLE);
    assert(ret == ESP_OK);
}

void IOExpander::lock()
{
    assert(xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE);
}

void IOExpander::unlock()
{
    assert(xSemaphoreGive(_mutex) == pdTRUE);
}

esp_err_t IOExpander::pullupMode(uint8_t pin, uint8_t mode) {
    if (pin < 1 || pin > 16) return ESP_ERR_INVALID_ARG;
    lock();
    if (mode == ON) {
        _pullupCache |= 1 << (pin - 1);
    } else {
        _pullupCache &= ~(1 << (pin -1));
    }
    esp_err_t ret = wordWrite(GPPUA, _pullupCache);
    unlock();
    return ret;
}

esp_err_t IOExpander::pinMode(uint8_t pin, uint8_t mode) 
{  
    // Accept the pin # and I/O mode
    if (pin < 1 || pin > 16) return ESP_ERR_INVALID_ARG;               // If the pin value is not valid (1-16) return, do nothing and return
    lock();
    if (mode == INPUT) {                          // Determine the mode before changing the bit state in the mode cache
        _modeCache |= 1 << (pin - 1);               // Since input = "HIGH", OR in a 1 in the appropriate place
    } else {
        _modeCache &= ~(1 << (pin - 1));            // If not, the mode must be output, so and in a 0 in the appropriate place
    }
    // Call the generic word writer with start register and the mode cache
    esp_err_t ret = wordWrite(IODIRA, _modeCache);
    unlock();
    return ret;
}

esp_err_t IOExpander::digitalWrite(uint8_t pin, uint8_t value)
{
    if (pin < 1 || pin > 16) return ESP_ERR_INVALID_ARG;
    lock();
    if (value) {
        _outputCache |= 1 << (pin - 1);
    } else {
        _outputCache &= ~(1 << (pin - 1));
    }
    esp_err_t ret = wordWrite(GPIOA, _outputCache);
    unlock();
    return ret;
}

esp_err_t IOExpander::wordWrite(uint8_t reg, unsigned int word)
{
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.flags=0;
    trans.addr = OPCODEW | (_address << 1);//Address
    uint8_t data[3];
    trans.tx_buffer = data;
    data[0] = reg;
    data[1] = (uint8_t) word;
    data[2] = (uint8_t) (word >> 8);
    trans.length = 3 * 8; //in bits    

    //Queue and wait for result, not thread safe
    return SPIBus::spi_device_transmit(_handle, &trans);
}

esp_err_t IOExpander::byteWrite(uint8_t reg, uint8_t value)
{
    
    //Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
    //function is finished because the SPI driver needs access to it even while we're already calculating the next line.
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.flags=0;
    trans.addr = OPCODEW | (_address << 1);//Address
    uint8_t data[2];
    trans.tx_buffer = data;
    data[0] = reg;
    data[1] = value;
    trans.length = 2  * 8; // in bits
    
    //Queue and wait for result
    return SPIBus::spi_device_transmit(_handle, &trans);
}

unsigned int IOExpander::digitalRead() 
{                                           // This function will read all 16 bits of I/O, and return them as a word in the format 0x(portB)(portA)
    unsigned int value = 0;                   // Initialize a variable to hold the read values to be returned

    spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.flags=0;
    trans.addr = OPCODER | (_address << 1);//Address + Read
    uint8_t tx_data[3] = {GPIOA, 0x00, 0x00};
    trans.tx_buffer = tx_data;
    trans.length = 3  * 8; // in bits
    uint8_t rx_data[3] = {0x00, 0x00, 0x00};
    trans.rx_buffer = rx_data;
    trans.rxlength = 3 * 8; // in bits

    esp_err_t ret = SPIBus::spi_device_transmit(_handle, &trans);
    assert(ret == ESP_OK);

    //printf("Tx data %2.2x %2.2x %2.2x \n", tx_data[0], tx_data[1], tx_data[2]);
    //printf("Rx data %2.2x %2.2x %2.2x \n", rx_data[0], rx_data[1], rx_data[2]);

    value = rx_data[1]; // Send any byte, the function will return the read value (register address pointer will auto-increment after write)
    value |= (rx_data[2] << 8); // Read in the "high byte" (portB) and shift it up to the high location and merge with the "low byte"

    return value; // Return the constructed word, the format is 0x(portB)(portA)
}

uint8_t IOExpander::byteRead(uint8_t reg)
{                                          
    // This function will read a single register, and return it
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.flags=0;
    trans.addr = OPCODER | (_address << 1);//Address + Read
    uint8_t tx_data[2] = {reg, 0x00};
    trans.tx_buffer = tx_data;
    trans.length = 2  * 8; // in bits
    uint8_t rx_data[2] = {0x00, 0x00};
    trans.rx_buffer = rx_data;
    trans.rxlength = 2 * 8; // in bits

    esp_err_t ret = SPIBus::spi_device_transmit(_handle, &trans);
    assert(ret == ESP_OK);

    return trans.rx_data[1];
}

uint8_t IOExpander::digitalRead(uint8_t pin) 
{                    // Return a single bit value, supply the necessary bit (1-16)
    if (pin < 1 || pin > 16) return 0x0;                    // If the pin value is not valid (1-16) return, do nothing and return
    return digitalRead() & (1 << (pin - 1)) ? HIGH : LOW;  // Call the word reading function, extract HIGH/LOW information from the requested pin
}

