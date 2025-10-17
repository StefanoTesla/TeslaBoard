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

    enum ActualCommand {
        Idle,
        Open,
        Close,
        Halt
    };


private:

    bool moduleEnable = false;
    bool rebootPending = false;
    
    Status status = NotPresent;
    ServoOutput cover;
    ActualCommand actualCmd = Idle;
    JsonDocument tmpCfg;


public:
    Cover(PWMManager* channelManager) : 
        chMgr(channelManager), 
        cover(channelManager)
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

    int getAngle(){ return cover.status();}
    
    void loop();
    
    bool isMoving();

    void updateStatus();
    void updateLastCommunication();

    Status getStatus() const;
    ActualCommand getActualCommand() const;

    void validateConfiguration(const JsonObject &obj, JsonObject response);
    void storeConfiguration(JsonObject toBeStored,const char* schema);
    void getConfiguration(JsonObject obj);



private:
    PWMManager* chMgr;
};

#endif