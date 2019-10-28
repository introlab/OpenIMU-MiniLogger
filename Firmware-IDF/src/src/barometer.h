#ifndef _BAROMETER_H_
#define _BAROMETER_H_

#include "defines.h"
#include "MPL115A2.h"

class Barometer
{
    public:
    static Barometer* instance();

    void getPT(float &p, float&t);

    private:
    static Barometer* _instance;
    Barometer();
    MPL115A2 _mpl115A2;
    TaskHandle_t _baroTaskHandle;
};

#endif
