#ifndef SDCARD_H
#define SDCARD_H

#include "FS.h"

class SDCard
{
public:
    SDCard();
    virtual ~SDCard();

    void begin();

    void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
    void toESP32();
    void toExternal();
};

namespace SDCardAction
{
    void SDToESP32();
    void SDToExternal();
}

#endif
