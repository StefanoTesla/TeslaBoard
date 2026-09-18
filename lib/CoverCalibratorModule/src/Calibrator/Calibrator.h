#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include <Arduino.h>
#include "IOManager.h"
#include <ArduinoJson.h>
#include <PWMManager.h>
#include <NVSManager.h>

class Calibrator {
public:
    enum Status {
        NotPresent,
        Off,
        NotReady,
        Ready,
        Unknow,
        Error
    };


private:
    PWMManager* chMgr;
    PWMOutput calibrator;
    
    bool moduleEnable = false;

    Status status = NotPresent;
    
    JsonDocument tmpCfg;

public:
    Calibrator(PWMManager* channelManager) : 
        chMgr(channelManager), 
        calibrator(channelManager)
        {}

    void begin(const JsonDocument& shutter);
    void loop();
    bool isEnable(){ return moduleEnable; }

    void setBrightness(int brightness){ calibrator.write(brightness); }
    int getBrightness(){ return calibrator.status(); }
    int getMaxBrightness(){ return calibrator.getMax(); }
    Status getStatus() const;

    void validateConfiguration(const JsonObject &obj, JsonObject response);
    void storeConfiguration(JsonObject toBeStored);
    void getConfiguration(JsonObject obj);


private:
    void updateStatus();
};

#endif