#ifndef BOARDMODULE_H
#define BOARDMODULE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>


#define BOARD_SCHEMA_VERSION 1
#define BOARD_SCHEMA_NAME "boardcfg"

class BoardModule {
public:
    BoardModule() = default;

    void begin(); 
    void loop();

    void getConfiguration(JsonObject dest);
    void validateConfiguration(const JsonObject &toBeValidated, JsonObject response);
    void storeConfiguration(JsonObject toBeStored);
    String getIdentifier(){ return identifier; }
    String getLocale(){ return locale; }

    unsigned long getUptime() { return upTime; }

private:

/* functions to handle the configuration */
    bool openNVS(bool readOnly);
    void closeNVS();
    bool initNVS();
    void updateNVS1();

    JsonDocument tmpCfg;
    Preferences nvs;
    enum PrefEnumStatus { CLOSED, OPEN_WRITE, OPEN_READOLNY };
    PrefEnumStatus nvsStatus = CLOSED;
    bool validConfig;
    bool rebootNeeded;

private:
    String identifier = "TeslaBoard";
    String locale = "en";

    unsigned long ackMillis = 0;
    unsigned long upTime = 0;
};

#endif