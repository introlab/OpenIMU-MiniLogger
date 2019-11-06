#include "configmanager.h"
//#include <cJSON.h>

ConfigManager* ConfigManager::_instance = nullptr;

ConfigManager* ConfigManager::instance()
{
    if (ConfigManager::_instance == nullptr)
        ConfigManager::_instance = new ConfigManager();
    return ConfigManager::_instance;
}


ConfigManager::ConfigManager()
{

}