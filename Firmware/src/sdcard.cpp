#include "sdcard.h"
#include "defines.h"
#include <ioexpander.h>
#include <FS.h>
#include <SD_MMC.h>
#include "adc.h"
#include "barometer.h"
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
    QueueHandle_t _powerQueue = NULL;
    QueueHandle_t _baroQueue = NULL;
    QueueHandle_t _timestampQueue = NULL;
    SemaphoreHandle_t _dataReadySemaphore = NULL;

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

    //TODO, unused for now
    ioExpander.pinMode(EXT_PIN04_SD_N_CD, INPUT);

    //PIN EXT_PIN05_SD_SEL (select 1=ESP32, 0 = USB2640)
    //EXT_PIN03_SD_N_ENABLED (output enable 0=NO SD Card)
    ioExpander.pinMode(EXT_PIN03_SD_N_ENABLED, OUTPUT);
    ioExpander.pinMode(EXT_PIN05_SD_SEL, OUTPUT);

    pinMode(0,INPUT_PULLUP);
    pinMode(2,INPUT_PULLUP);
    pinMode(4,INPUT_PULLUP);
    pinMode(12,INPUT_PULLUP);
    pinMode(13,INPUT_PULLUP);

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

bool SDCard::mount()
{
    if(!SD_MMC.begin("/sdcard", false)) {
       Serial.println("Card Mount Failed");
       return false;
   }

   uint8_t cardType = SD_MMC.cardType();

   if(cardType == CARD_NONE){
       Serial.println("No SD_MMC card attached");
       return false;
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

   return true;
}

void SDCard::toESP32()
{
    Serial.println("SD to ESP32");

    //TODO - DL FIX PINS
    SD_MMC.end();

    //Select ESP32 for SD
    ioExpander.digitalWrite(EXT_PIN05_SD_SEL,HIGH);

    //Output enable (inverted)
    ioExpander.digitalWrite(EXT_PIN03_SD_N_ENABLED,HIGH);
    delay(500);
    ioExpander.digitalWrite(EXT_PIN03_SD_N_ENABLED,LOW);
    delay(500);

    // Mount SD Card
    if(mount()) {
        listDir(SD_MMC, "/", 0);
    }
}

void SDCard::toExternal()
{
    Serial.println("SD to external");

    SD_MMC.end();

    //Select USB for SD
    ioExpander.digitalWrite(EXT_PIN05_SD_SEL,LOW);

    //Output enable (inverted)
    ioExpander.digitalWrite(EXT_PIN03_SD_N_ENABLED,HIGH);
    delay(500);
    ioExpander.digitalWrite(EXT_PIN03_SD_N_ENABLED,LOW);
    delay(500);
}

void SDCard::startLog()
{
    File latest;
    int logNo;
    char c;
    String str = "";

    if(_logTask == NULL) {
        toESP32();

        if(!SD_MMC.exists("/latest.txt"))
        {
            latest = SD_MMC.open("/latest.txt", FILE_WRITE);
            latest.print(0, DEC);
            latest.close();
        }

        latest = SD_MMC.open("/latest.txt", FILE_READ);
        Serial.println("Reading latest log file number");
        while(latest.available()) {
            c = latest.read();
            str += c;
            Serial.print(c);
        }
        Serial.println("");
        logNo = str.toInt();
        latest.close();

        latest = SD_MMC.open("/latest.txt", FILE_WRITE);
        latest.print(++logNo);
        latest.close();

        _logFile = SD_MMC.open("/" + String(logNo) + ".oimu", FILE_WRITE);
        if(_logFile) {
            _logFile.write('h');
            startTimestamp();
            xTaskCreatePinnedToCore(&logToFile, "SD card log", 2048, NULL, 5, &_logTask, 1);
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

void SDCard::setPowerQueue(QueueHandle_t queue)
{
    _powerQueue = queue;
}

void SDCard::setBarometerQueue(QueueHandle_t queue)
{
    _baroQueue = queue;
}

void SDCard::setDataReadySemaphore(SemaphoreHandle_t semaphore)
{
    _dataReadySemaphore = semaphore;
}

void SDCard::startTimestamp()
{
    _timestampQueue = xQueueCreate(20, sizeof(time_t));
    xTaskCreatePinnedToCore(&generateTimestamp, "SD card log", 2048, NULL, 15, &_timestampTask, 1);
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
        Serial.println("logToFile Task started.");


        gpsDataSendable_t gpsSendable;
        imuDataSendable_t imuSendable;
        timestampSendable_t timestamp;

        int imu_cnt = 0;
        int gps_cnt = 0;
        int power_cnt = 0;
        int baro_cnt = 0;

        while(1) {

            //Should wake at every second
            xSemaphoreTake(_dataReadySemaphore, portMAX_DELAY);
            //Serial.println("_dataReadySemaphore");

            // Log timestamp
            if(xQueueReceive(_timestampQueue, &timestamp.data, 0) == pdTRUE) {
                _logFile.write('t');
                _logFile.write(timestamp.bytes, sizeof(time_t));


                int nb_sec = timestamp.bytes[3]*256*256*256 + timestamp.bytes[2]*256*256+ timestamp.bytes[1]*256 + timestamp.bytes[0];
                Serial.printf("TS %d\n",nb_sec );

                // Serial.printf("WR Timestamp i: %i g: %i p: %i b: %i\n", imu_cnt
                //         , gps_cnt, power_cnt, baro_cnt);
                imu_cnt = 0;
                gps_cnt = 0;
                power_cnt = 0;
                baro_cnt = 0;
            }

            // Log imu
            if(_imuQueue != NULL) {
                imuData_ptr imuDataPtr = NULL;
                if(xQueueReceive(_imuQueue, &imuDataPtr, 0) == pdTRUE) {
                    _logFile.write('i');
                    _logFile.write((uint8_t*) imuDataPtr, sizeof(imuData_t));
                    delete imuDataPtr;
                    imu_cnt++;
                }
            }

            // Log GPS
            if(_gpsQueue != NULL) {
                gpsData_ptr gpsDataPtr = NULL;

                if(xQueueReceive(_gpsQueue, &gpsDataPtr, 0) == pdTRUE) {
                    //Serial.printf("LOG GPS : %i %f %f %f\n", gpsDataPtr->fix,
                    //        gpsDataPtr->latitude, gpsDataPtr->longitude, gpsDataPtr->altitude);
                    _logFile.write('g');
                    _logFile.write((uint8_t*)gpsDataPtr, sizeof(gpsData_t));
                    free(gpsDataPtr);
                    gps_cnt++;
                }
            }

            // Log POWER
            if(_powerQueue != NULL) {
                powerData_ptr powerDataPtr = NULL;
                if(xQueueReceive(_powerQueue, &powerDataPtr, 0) == pdTRUE) {
                    _logFile.write('p');
                    _logFile.write((uint8_t*) powerDataPtr, sizeof(powerData_t));
                    free(powerDataPtr);
                    power_cnt++;
                }
            }

            // Log Barometer
            if(_baroQueue != NULL) {
                baroData_ptr baroDataPtr = NULL;
                if(xQueueReceive(_baroQueue, &baroDataPtr, 0) == pdTRUE) {
                    _logFile.write('b');
                    _logFile.write((uint8_t*) baroDataPtr, sizeof(baroData_t));
                    free(baroDataPtr);
                    baro_cnt++;
                }
            }
        }
    }

    void generateTimestamp(void *pvParameters)
    {
        TickType_t lastGeneration = xTaskGetTickCount();
        time_t now;

        while(1) {
            vTaskDelayUntil(&lastGeneration, 1000 / portTICK_RATE_MS);
            time(&now);
            if(xQueueSend(_timestampQueue, &now, 0) == pdTRUE) {
                //Serial.println("giving data ready");
                xSemaphoreGive(_dataReadySemaphore);
            }
        }
    }
}
