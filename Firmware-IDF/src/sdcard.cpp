#include "sdcard.h"
#include "ioexpander.h"

SDCard* SDCard::_instance = NULL;

SDCard* SDCard::instance()
{
    if (SDCard::_instance == NULL)
        SDCard::_instance = new SDCard();
    return SDCard::_instance;
}


SDCard::SDCard()
{
    //TODO, unused for now
    IOExpander::instance().pinMode(EXT_PIN04_SD_N_CD, INPUT);

    //PIN EXT_PIN05_SD_SEL (select 1=ESP32, 0 = USB2640)
    //EXT_PIN03_SD_N_ENABLED (output enable 0=NO SD Card)
    IOExpander::instance().pinMode(EXT_PIN03_SD_N_ENABLED, OUTPUT);
    IOExpander::instance().pinMode(EXT_PIN05_SD_SEL, OUTPUT);

    //SD-MMC all pins as inputs before initializing module
    setup_gpio(0);
    setup_gpio(2);
    setup_gpio(4);
    setup_gpio(12);
    setup_gpio(13);

    _card = NULL;

    //Setup host
    _host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    _slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    

    toESP32();
    //toExternal();
}

SDCard::~SDCard()
{

}

void SDCard::setup_gpio(int pin)
{
    gpio_config_t io_conf;
    //interrupt on falling edge
    io_conf.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_DISABLE;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO33
    io_conf.pin_bit_mask =  (1ULL << pin);
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t) 0;
    //enable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t) 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

}

void SDCard::toESP32()
{
    printf("SD to ESP32 \n");
    unmount();

    //Select ESP32 for SD
    IOExpander::instance().digitalWrite(EXT_PIN05_SD_SEL,HIGH);

    //Output enable (inverted)
    IOExpander::instance().digitalWrite(EXT_PIN03_SD_N_ENABLED,HIGH);
    vTaskDelay(500 / portTICK_RATE_MS);
    IOExpander::instance().digitalWrite(EXT_PIN03_SD_N_ENABLED,LOW);
    vTaskDelay(500 / portTICK_RATE_MS);

    // Mount SD Card
    if(mount()) {
    //    listDir(SD_MMC, "/", 0);
    }
}

void SDCard::toExternal()
{
    printf("SD to external\n");

    unmount();

    //Select USB for SD
    IOExpander::instance().digitalWrite(EXT_PIN05_SD_SEL,LOW);

    //Output enable (inverted)
    IOExpander::instance().digitalWrite(EXT_PIN03_SD_N_ENABLED,HIGH);
    vTaskDelay(500 / portTICK_RATE_MS);
    IOExpander::instance().digitalWrite(EXT_PIN03_SD_N_ENABLED,LOW);
    vTaskDelay(500 / portTICK_RATE_MS);
}


bool SDCard::mount()
{
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config;
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 5;
    //mount_config.allocation_unit_size = 16 * 1024;


    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &_host, &_slot_config, &mount_config, &_card);
    if (ret == ESP_OK)
    {
        //Print card info
        sdmmc_card_print_info(stdout, _card);
        return true;
    }

    return false;
}

void SDCard::unmount()
{
    esp_vfs_fat_sdmmc_unmount();
}