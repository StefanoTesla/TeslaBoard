#ifndef DOME_H
#define DOME_H

#include <Arduino.h>
#include "Shutter/Shutter.h"
#include <ArduinoJson.h>
#include <Preferences.h>

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
    String getIdentifier(){ return identifier; }
    unsigned int uiOrder;

private:

/* functions to handle the configuration */
    void openNFS(bool readOnly);
    void initNVS(Preferences pref);
    void updateNVS1(Preferences pref);



    JsonDocument tmpCfg;
    bool moduleEnable;
    bool validConfig;


private:
    String identifier = "Dome";
};

#endif