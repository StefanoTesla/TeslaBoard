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
    PWMManager* chMgr;
    String identifier = "CoverCalibrator";
};

#endif