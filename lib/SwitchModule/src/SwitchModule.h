#ifndef DOME_H
#define DOME_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <PWMManager.h>
#include <IOManager.h>
#include <DigitalInput/DigitalInput.h>
#include <DigitalOutput/DigitalOutput.h>
#include <PWM/PWMOutput.h>
#include <DigitalInput/DigitalInput.h>
#include "esp_log.h"

#define SWITCH_SCHEMA_VERSION 1
#define SWITCH_SCHEMA_NAME "switchcfg"
#define SWITCH_MAX_SWITCHES 21

class SwitchModule {
public:
public:
    SwitchModule(PWMManager* channelManager) : chMgr(channelManager) {}

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
    void initNVS();
    void updateNVS1();

    JsonDocument tmpCfg;
    bool moduleEnable;
    bool validConfig;


private:
    PWMManager* chMgr;
    String identifier = "Switch";
    IOBase* Switches[SWITCH_MAX_SWITCHES] = {nullptr};

};

#endif