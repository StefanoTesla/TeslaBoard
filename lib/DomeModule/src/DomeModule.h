#ifndef DOME_H
#define DOME_H

#include <Arduino.h>
#include "Shutter/Shutter.h"
#include <ArduinoJson.h>
#include <Preferences.h>
#include "esp_log.h"

#define DOME_SCHEMA_VERSION 1
#define DOME_SCHEMA_NAME "domecfg"

class DomeModule {
public:
    Shutter shutter;

    DomeModule() = default;
    void begin(); 
    bool isEnable();
    void loop();


    void getConfiguration(JsonObject dest);
    void validateConfiguration(const JsonObject &toBeValidated, JsonObject response);
    void storeConfiguration(JsonObject toBeStored);
    bool needReboot() { return rebootNeeded; }
    String getIdentifier(){ return identifier; }
    unsigned int uiOrder;

private:

/* functions to handle the configuration */
    enum PrefEnumStatus { CLOSED, OPEN_WRITE, OPEN_READOLNY };
    PrefEnumStatus nvsStatus = CLOSED;
    bool openNVS(bool readOnly);
    void closeNVS();
    bool initNVS();
    void updateNVS1();

    bool moduleEnable = false;
    bool validConfig = false;
    bool rebootNeeded = false;


private:
    String identifier = "Dome";
    Preferences nvs;
};

#endif