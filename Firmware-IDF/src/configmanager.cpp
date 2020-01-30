#include "configmanager.h"
#include <cJSON.h>

ConfigManager* ConfigManager::_instance = nullptr;

ConfigManager* ConfigManager::instance()
{
    if (ConfigManager::_instance == nullptr)
        ConfigManager::_instance = new ConfigManager();
    return ConfigManager::_instance;
}

ConfigManager::ConfigManager()
{
    //Make a valid default configuration
    _imuConfig = {10,500,8};
    _openTeraConfig = {"MiniLogger", "localhost", 4040,""};
    _wifiConfig={"wifissid", "wifipassword"};

    load_configuration();
}

IMUconfig_Sd ConfigManager::getIMUConfig()
{
    //Return a copy of the configuration
    return _imuConfig;
}

void ConfigManager::setIMUConfig(const IMUconfig_Sd &config)
{
    _imuConfig = config;
}

OpenTeraConfig_Sd ConfigManager::getOpenTeraConfig()
{
    //Return a copy of the configuration
    return _openTeraConfig;
}

void ConfigManager::setOpenTeraConfig(const OpenTeraConfig_Sd &config)
{
    _openTeraConfig = config;
}

WiFiConfig_Sd ConfigManager::getWiFiConfig()
{
    //Return a copy of the configuration
    return _wifiConfig;
}

void ConfigManager::setWiFiConfig(const WiFiConfig_Sd &config)
{
    _wifiConfig = config;
}

std::string ConfigManager::json_configuration()
{
    
    cJSON *root=cJSON_CreateObject();

    //IMU
    cJSON_AddNumberToObject(root, "samplerate", _imuConfig.IMUSampleRate);
    cJSON_AddNumberToObject(root, "setupaccel", _imuConfig.IMUAcellRange);
    cJSON_AddNumberToObject(root, "setupgyro", _imuConfig.IMUGyroRange);

    //OpenTera
    cJSON_AddStringToObject(root, "devicename", _openTeraConfig.deviceName.c_str());
    cJSON_AddStringToObject(root, "openteraserver", _openTeraConfig.openTeraServerName.c_str());
    cJSON_AddNumberToObject(root, "openteraport", _openTeraConfig.openTeraServerPort);
    cJSON_AddStringToObject(root, "openteratoken", _openTeraConfig.openTeraToken.c_str());

    //WiFi
    cJSON_AddStringToObject(root, "wifissid", _wifiConfig.ssid.c_str());
    cJSON_AddStringToObject(root, "wifipassword", _wifiConfig.password.c_str());

    std::string result(cJSON_Print(root));

    //Free memory
    cJSON_free(root);

    return result;
}

void ConfigManager::print_configuration()
{
    printf("Configuration: %s\n", json_configuration().c_str());
}

bool ConfigManager::load_configuration(const std::string &path)
{
    printf("ConfigManager::load_configuration : %s \n", path.c_str());

    struct stat stt;
    if (stat(path.c_str(), &stt) != 0)
    {
        printf("No folder found, Creating folder with default configuration\n");
        create_configuration_folder();
        return false;
    }
    else   
    {
        FILE* f = fopen(path.c_str(),"r");

        if (f==NULL)
        {
            printf("Can't open the directory\n");
            return false;
        }

        //Go to end of file
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        printf("JSON file size returns : %li\n", size);

        //Go to begin of file
        fseek(f, 0 , SEEK_SET);
       
        //Allocate string with null character at the end 
        char* json_string = (char*) malloc(size + 1);
        memset(json_string,0, size +1);

        //Read the complete file
        fread(json_string, 1, size, f);
        //printf("json : %s \n", json_string);

        //Parse JSON
        cJSON *root = cJSON_Parse(json_string);

        //Structure is initialized with default values. Replace value if found in JSON file

        //IMU Config
        if (cJSON_HasObjectItem(root, "samplerate"))
            _imuConfig.IMUSampleRate = cJSON_GetObjectItem(root,"samplerate")->valueint;
        if (cJSON_HasObjectItem(root, "setupaccel"))
            _imuConfig.IMUAcellRange = cJSON_GetObjectItem(root,"setupaccel")->valueint;
        if (cJSON_HasObjectItem(root, "setupgyro"))
            _imuConfig.IMUGyroRange = cJSON_GetObjectItem(root,"setupgyro")->valueint;

        //OpenTera Config
        if (cJSON_HasObjectItem(root, "devicename"))
            _openTeraConfig.deviceName = std::string(cJSON_GetObjectItem(root,"devicename")->valuestring);
        if (cJSON_HasObjectItem(root, "openteraserver"))
            _openTeraConfig.openTeraServerName = std::string(cJSON_GetObjectItem(root,"openteraserver")->valuestring);
        if (cJSON_HasObjectItem(root, "openteraport"))
            _openTeraConfig.openTeraServerPort = cJSON_GetObjectItem(root,"openteraport")->valueint; 
        if (cJSON_HasObjectItem(root, "openteratoken"))
            _openTeraConfig.openTeraToken = std::string(cJSON_GetObjectItem(root,"openteratoken")->valuestring); 

        //WiFi Config
        if (cJSON_HasObjectItem(root, "wifissid"))
            _wifiConfig.ssid = std::string(cJSON_GetObjectItem(root,"wifissid")->valuestring);
        if (cJSON_HasObjectItem(root, "wifipassword"))
            _wifiConfig.password = std::string(cJSON_GetObjectItem(root,"wifipassword")->valuestring);    

        //Free memory
        cJSON_free(root);
        free(json_string);

        //Close file
        fclose(f);
    }

    return true;
}

 bool ConfigManager::save_configuration(const std::string &path)
 {
    printf("ConfigManager::save_configuration : %s \n", path.c_str());

    struct stat stt;
    if (stat(path.c_str(), &stt) != 0)
    {
        printf("No folder found\n");
        create_configuration_folder();
        return false;
    }
    else 
    {
        FILE* f = fopen(path.c_str(),"w");

        if (f==NULL)
        {
            printf("Can't open the directory\n");
            return false;
        }
        //Get JSON string
        std::string json_conf = json_configuration();

        //Write to file
        fwrite(json_conf.c_str(), json_conf.size(), 1, f);

        //Close file
        fclose(f);
    }

    return true;
 }

bool ConfigManager::create_configuration_folder(const std::string &path)
 {
    if(mkdir("/sdcard/ParameterFolder",0007)==-1)
    {
        printf("Problem Creating Folder\n");
        return false;
    }

    FILE* f = fopen(path.c_str(),"w");

    if (f==NULL)
    {
        printf("Can't open the directory\n");
        return false;
    }
    //Get JSON string
    std::string json_conf = json_configuration();

    //Write to file
    fwrite(json_conf.c_str(), json_conf.size(), 1, f);

    //Close file
    fclose(f);

    return true;
 }
