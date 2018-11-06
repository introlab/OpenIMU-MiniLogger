#ifndef ACTIONS_H
#define ACTIONS_H

namespace Actions
{
    extern void SDToESP32();
    extern void SDToExternal();

    extern void LaunchMAX();
    // extern void IMUStartSerial();
    // extern void IMUStopSerial();

    extern void IMUStartSD();
    extern void IMUStopSD();

    extern void Shutdown();
}

#endif
