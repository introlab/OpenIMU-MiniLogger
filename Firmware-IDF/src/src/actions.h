#ifndef ACTIONS_H
#define ACTIONS_H

namespace Actions
{
    extern void SDToESP32();
    extern void SDToExternal();

    // extern void IMUStartSerial();
    // extern void IMUStopSerial();

    extern void IMUStartSD();
    extern void IMUStopSD();

    extern void ChangeSampleRate();

    extern void Shutdown();
}

#endif
