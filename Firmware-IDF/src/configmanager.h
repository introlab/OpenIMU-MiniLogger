#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include <string>

/* Global configuration will be stored here. Can be serialized to JSON */
class ConfigManager {
    public: 

    //Singleton
    static ConfigManager* instance();

    bool load_configuration(const std::string &path = "/sdcard/ParameterFolder/StartingParameter.json");
    bool save_configuration(const std::string &path = "/sdcard/ParameterFolder/StartingParameter.json");
    bool create_configuration_folder(const std::string &path = "/sdcard/ParameterFolder/StartingParameter.json");

    std::string json_configuration();

    void print_configuration();

    IMUconfig_Sd getIMUConfig();

    void setIMUConfig(const IMUconfig_Sd &config);

    OpenTeraConfig_Sd getOpenTeraConfig();

    void setOpenTeraConfig(const OpenTeraConfig_Sd &config);

    WiFiConfig_Sd getWiFiConfig();

    void setWiFiConfig(const WiFiConfig_Sd &config);

    private:

    ConfigManager();

    static ConfigManager* _instance;

    //IMU configuration
    IMUconfig_Sd _imuConfig;
    //OpenTera configuration
    OpenTeraConfig_Sd _openTeraConfig;
    //WiFi configuration
    WiFiConfig_Sd _wifiConfig;
};

#endif
