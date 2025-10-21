#ifndef COVER_H
#define COVER_H

#include <Arduino.h>
#include "IOManager.h"
#include <ArduinoJson.h>
#include <Preferences.h>
#include <PWMManager.h>

class Cover {
public:
    enum Status {
        NotPresent,
        Closed,
        Moving,
        Opened,
        Unknown,
        Error
    };

private:
    PWMManager* chMgr;
    ServoOutput servo;
    bool moduleEnable = false;
    bool rebootPending = false;
    unsigned int openPosition;
    unsigned int closePosition;
    Status status = NotPresent;
    
    JsonDocument tmpCfg;


public:
    Cover(PWMManager* channelManager) : 
        chMgr(channelManager), 
        servo(channelManager)
        {}


    void begin(JsonDocument shutter);
    bool isEnable(){ return moduleEnable; }

    bool isOpen();
    bool canOpen();
    void open();

    bool isClosed();
    bool canClose();
    void close();

    void halt();

    int getPosition(){ return servo.status();}
    unsigned int getOpenPosition(){return openPosition;}
    unsigned int getclosePosition(){return closePosition;}
    void setOpenPosition(unsigned int _value);
    void setClosePosition(unsigned int _value);

    void loop();
    
    bool isMoving();

    void updateStatus();
    void updateLastCommunication();

    Status getStatus() const;

    void validateConfiguration(const JsonObject &obj, JsonObject response);
    void storeConfiguration(JsonObject toBeStored,const char* schema);
    void getConfiguration(JsonObject obj);


};

#endif