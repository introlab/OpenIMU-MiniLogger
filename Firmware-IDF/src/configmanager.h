#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

#include "defines.h"


class ConfigManager {
    public: 

    //Singleton
    static ConfigManager* instance();

    private:

    ConfigManager();

    static ConfigManager* _instance;
};

#endif
