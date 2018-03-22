#include <Arduino.h>

#include "SparkFunMPU9250-DMP.h"

#include "display.h"
#include "menu.h"
#include "sdcard.h"
#include "buttons.h"

Display display;
Menu menu;
SDCard sdCard;
Buttons buttons;

void setup() {

    // Start serial
    Serial.begin(115200);
    delay(3000);

    // Start display
    Serial.println("Initializing display...");
    delay(1000);

    // Show menu and start reading buttons
    display.begin();
    display.showMenu(&menu);
    buttons.begin();

    Serial.println("Display Ready");

    // Initialize SD-card
    sdCard.begin();

    Serial.println("System ready");
}

void loop() {

    while(buttons.getActionCtn() > 0) {
        menu.action();
        buttons.decrementActionCtn();
    }

    while(buttons.getPreviousCtn() > 0) {
        menu.previous();
        buttons.decrementPreviousCtn();
    }

    while(buttons.getNextCtn() > 0) {
        menu.next();
        buttons.decrementNextCtn();
    }

    display.updateMenu(&menu);
}
