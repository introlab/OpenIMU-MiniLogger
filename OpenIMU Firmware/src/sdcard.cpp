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
