#ifndef COVERCMODULE_H
#define COVERCMODULE_H

#include <Arduino.h>
#include "Cover/Cover.h"
#include "Calibrator/Calibrator.h"
#include <ArduinoJson.h>
#include <Preferences.h>
#include <PWMManager.h>

#define COVERC_SCHEMA_VERSION 1
#define COVERC_SCHEMA_NAME "cccfg"

class CoverCalibratorModule {
public:
    CoverCalibratorModule(PWMManager* channelManager) :
        chMgr(channelManager),       
        cover(channelManager),       
        calibrator(channelManager)      
    {}
    Cover cover;
    Calibrator calibrator;


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
    Preferences nvs;
    enum PrefEnumStatus { CLOSED, OPEN_WRITE, OPEN_READOLNY };
    PrefEnumStatus nvsStatus = CLOSED;
    bool openNVS(bool readOnly);
    void closeNVS();
    bool initNVS();
    void updateNVS1();

    JsonDocument tmpCfg;
    bool moduleEnable = false;
    bool validConfig = false;
    bool rebootNeeded = false;

private:
    PWMManager* chMgr;
    String identifier = "CoverCalibrator";
};

#endif