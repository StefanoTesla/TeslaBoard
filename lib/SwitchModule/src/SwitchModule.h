#ifndef SWITCH_H
#define SWITCH_H

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
    enum Type {
        NotPresent,
        Input,
        Output,
        PWM,
        Servo
    };

public:
    SwitchModule(PWMManager* channelManager) : chMgr(channelManager) {}

    void begin(); 
    bool isEnable() { return moduleEnable; };
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

    JsonObject tmpLoad;
    JsonDocument tmpCfg;
    bool moduleEnable;
    bool validConfig;


private:
    PWMManager* chMgr;
    String identifier = "Switch";
    IOBase* Switches[SWITCH_MAX_SWITCHES] = {nullptr};
    unsigned int configuredSwitches;

};

#endif