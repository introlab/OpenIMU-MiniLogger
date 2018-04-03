#include "sdcard.h"

#include <FS.h>
#include <SD_MMC.h>

#include <vfs_api.h>

extern "C" {
    #include <sys/unistd.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <esp_vfs_fat.h>
    #include <driver/sdmmc_host.h>
    #include <driver/sdmmc_defs.h>
    #include <sdmmc_cmd.h>
}

#include <ff.h>
#include <Arduino.h>

namespace
{
    File _logFile;
    QueueHandle_t _imuQueue = NULL;
    QueueHandle_t _gpsQueue = NULL;
    QueueHandle_t _timestampQueue = NULL;

    TaskHandle_t _logTask = NULL;
    TaskHandle_t _timestampTask = NULL;

    void logToFile(void *pvParameters);
    void generateTimestamp(void *pvParameters);
}

SDCard::SDCard()
{

}

SDCard::~SDCard()
{

}

void SDCard::begin()
{
    //PIN 25 (select 1=ESP32, 0 = USB2640) et 26 (output enable 0=PAS DE CARTE SD) 1

    pinMode(5, OUTPUT);
    pinMode(25,OUTPUT);
    pinMode(26,OUTPUT);

    pinMode(0,INPUT_PULLUP);
    pinMode(2,INPUT_PULLUP);
    pinMode(4,INPUT_PULLUP);
    pinMode(12,INPUT_PULLUP);
    pinMode(13,INPUT_PULLUP);

    //Select ESP32 for SD
    digitalWrite(25,HIGH);
    digitalWrite(5, HIGH);

    //Output enable
    digitalWrite(26,LOW);
    delay(500);
    digitalWrite(26,HIGH);
    delay(500);

    toESP32();
}


void SDCard::listDir(fs::FS &fs, const char * dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void SDCard::toESP32()
{
    Serial.println("SD to ESP32");

    //Select ESP32 for SD
    digitalWrite(25,HIGH);
    digitalWrite(5, HIGH);

    // Mount SD Card

    if(!SD_MMC.begin("/sdcard", false)) {
       Serial.println("Card Mount Failed");
       return;
   }

   uint8_t cardType = SD_MMC.cardType();

   if(cardType == CARD_NONE){
       Serial.println("No SD_MMC card attached");
       return;
   }

   Serial.print("SD_MMC Card Type: ");
   if(cardType == CARD_MMC){
       Serial.println("MMC");
   } else if(cardType == CARD_SD){
       Serial.println("SDSC");
   } else if(cardType == CARD_SDHC){
       Serial.println("SDHC");
   } else {
       Serial.println("UNKNOWN");
   }

   uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
   Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);


   listDir(SD_MMC, "/", 0);
}

void SDCard::toExternal()
{
    Serial.println("SD to external");

    SD_MMC.end();

    digitalWrite(5, LOW);
    digitalWrite(25, LOW);
}

void SDCard::startLog()
{
    File latest;
    int logNo;
    char c;
    String str;

    if(_logTask == NULL) {
        toESP32();

        if(!SD_MMC.exists("/latest.txt"))
        {
            latest = SD_MMC.open("/latest.txt", FILE_WRITE);
            latest.print(0, DEC);
            latest.close();
        }

        latest = SD_MMC.open("/latest.txt", FILE_READ);
        c = latest.read();
        while(c != -1) {
            str += c;
            c = latest.read();
        }
        logNo = str.toInt();
        latest.close();

        latest = SD_MMC.open("/latest.txt", FILE_WRITE);
        latest.print(logNo);
        latest.close();

        _logFile = SD_MMC.open("/" + String(logNo) + ".oimu", FILE_WRITE);
        if(_logFile) {
            _logFile.write('h');
            startTimestamp();
            xTaskCreate(&logToFile, "SD card log", 2048, NULL, 5, &_logTask);
        }

        else {
            Serial.println("Failed to open log file");
        }
    }
}

void SDCard::stopLog()
{
    if(_logTask != NULL) {
        vTaskDelete(_logTask);
        stopTimestamp();
        _logTask = NULL;
        _logFile.close();
    }
}

void SDCard::setIMUQueue(QueueHandle_t queue)
{
    _imuQueue = queue;
}

void SDCard::setGPSQueue(QueueHandle_t queue)
{
    _gpsQueue = queue;
}

void SDCard::startTimestamp()
{
    _timestampQueue = xQueueCreate(20, sizeof(time_t));
    xTaskCreate(&generateTimestamp, "SD card log", 2048, NULL, 5, &_timestampTask);
}

void SDCard::stopTimestamp()
{
    vTaskDelete(_timestampTask);
    vQueueDelete(_timestampQueue);
}

namespace
{
    void logToFile(void *pvParameters)
    {
        imuData_ptr imuMeasure;
        gpsDataSendable_t gpsSendable;
        imuDataSendable_t imuSendable;
        timestampSendable_t timestamp;

        while(1) {
            // Log timestamp
            if(xQueueReceive(_timestampQueue, &timestamp.data, 0) == pdTRUE) {
                _logFile.write('t');
                _logFile.write(timestamp.bytes, sizeof(time_t));
            }

            // Log imu
            if(_imuQueue != NULL) {
                if(xQueueReceive(_imuQueue, &imuMeasure, 0) == pdTRUE) {
                    imuSendable.data = *imuMeasure;
                    _logFile.write('i');
                    _logFile.write(imuSendable.bytes, sizeof(imuData_t));
                    delete imuMeasure;
                }
            }

            // Log GPS
            if(_gpsQueue != NULL) {
                if(xQueueReceive(_gpsQueue, &imuSendable.data, 0) == pdTRUE) {
                    _logFile.write('g');
                    _logFile.write(gpsSendable.bytes, sizeof(gpsData_t));
                }
            }
            vTaskDelay(10/portTICK_RATE_MS);
        }
    }

    void generateTimestamp(void *pvParameters)
    {
        TickType_t lastGeneration = xTaskGetTickCount();
        time_t now;

        while(1) {
            vTaskDelayUntil(&lastGeneration, 1000 / portTICK_RATE_MS);
            time(&now);
            xQueueSend(_timestampQueue, &now, 0);
        }
    }
}
