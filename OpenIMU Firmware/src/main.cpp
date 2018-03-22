#include <Arduino.h>

#include "Adafruit_MCP23008.h"
#include "SparkFunMPU9250-DMP.h"

#include "display.h"
#include "menu.h"

#include "FS.h"
#include "SD_MMC.h"

#include "vfs_api.h"

extern "C" {
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
}
#include "ff.h"

Display display;
Menu menu;
Adafruit_MCP23008 mcp;

// Flags for buttons read
TickType_t lastButtonRead;

int actionCtn = 0;
int previousCtn = 0;
int nextCtn = 0;

bool lastAction = false;
bool lastPrevious = false;
bool lastNext = false;

void readButton(void *pvParameters) {
    bool action, previous, next;

    lastButtonRead = xTaskGetTickCount();

    while(1) {
        vTaskDelayUntil(&lastButtonRead, 100 / portTICK_RATE_MS);

        action = mcp.digitalRead(2) != 0;
        previous = mcp.digitalRead(6) != 0;
        next = mcp.digitalRead(7) != 0;

        if(action && action != lastAction) {
            actionCtn++;
        }

        if(previous && previous != lastPrevious) {
            previousCtn++;
        }

        if(next && next != lastNext) {
            nextCtn++;
        }

        lastAction = action;
        lastPrevious = previous;
        lastNext = next;
    }
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels)
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

void SDToESP32()
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

void SDToExternal()
{
    Serial.println("SD to external");

    SD_MMC.end();

    digitalWrite(5, LOW);
    digitalWrite(25, LOW);
}

void setup() {

    // Start serial
    Serial.begin(115200);
    delay(3000);

    // Enable IO expander
    mcp.begin();

    mcp.pinMode(2, INPUT);
    mcp.pullUp(2, HIGH);

    mcp.pinMode(6, INPUT);
    mcp.pullUp(6, HIGH);

    mcp.pinMode(7, INPUT);
    mcp.pullUp(7, HIGH);

    // Start display
    Serial.println("Initializing display...");
    delay(1000);

    display.begin();
    display.showMenu(&menu);
    Serial.println("Display Ready");

    // Show menu and start reading buttons
    xTaskCreate(&readButton, "Buttons", 2048, NULL, 5, NULL);

    // Initialize SD-card
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

    SDToESP32();

    Serial.println("System ready");
}

void loop() {

    while(actionCtn > 0) {
        menu.action();
        actionCtn--;
    }

    while(previousCtn > 0) {
        menu.previous();
        previousCtn--;
    }

    while(nextCtn > 0) {
        menu.next();
        nextCtn--;
    }

    display.updateMenu(&menu);
}
