#ifndef SHUTTER_H
#define SHUTTER_H

#include <Arduino.h>
#include "IOManager.h"
#include <ArduinoJson.h>
#include <Preferences.h>
#include "esp_log.h"

class Shutter {
public:
    enum Status {
        Opened,
        Closed,
        Opening,
        Closing,
        Error
    };

    enum ActualCommand {
        Idle,
        Open,
        Close,
        Halt
    };


private:
    Status status = Error;
    DigitalInput OpenSensor;
    DigitalInput CloseSensor;
    DigitalOutput StartOpen;
    DigitalOutput HaltClose;

    ActualCommand actualCmd = Idle;

    enum DriverType{
        GateController =1,
        DirectionalOutput,
        StartAndDirectionOutput,
    };

    DriverType driverType;

    enum outputDirection{
      Stop,
      goToOpen,
      goToClose,
      safeStop  
    };

    outputDirection direction;

    enum CycleStep{
        WaitForACommand,
        WaitSensorLoosing,
        CheckToBeArrived,
        ArrivedToOpenDestination,
        ArrivedToCloseDestination,
        FinalReset,
        PPSendHaltSignal,
        PPResetHaltSignal,
        PPWaitBeforeSendANewCommand,
        PPSendTheCommandAgain,

        //always latest steps
        HaltBegin,
        HaltWait,
        HaltFinalStep
    };

    CycleStep actualStep = WaitForACommand;

    struct autoClose{
        bool enable = false;
        unsigned long waitingTime = 30; //[min] during the setup is multiplied by 60000 to conver minutes to ms
        unsigned long remaningTime; //[s] return the seconds before the roof will be closed automatically
        unsigned long lastCommunication; //[ms] the last comunication millis from api or alpaca
    };

    unsigned long ackTimeout = 0;

    unsigned long startTravelMillis;
    unsigned long travelTime;
    unsigned long travelTOUT = 20;

    bool retry = false; //used only for gate controller board

    bool moduleEnable = false;

    autoClose autoClose;
    
    JsonDocument tmpCfg;


public:
    Shutter() = default;

    void begin(JsonDocument shutter);
    bool isEnable(){ return moduleEnable; }

    bool isOpen();
    bool canOpen();
    void open();

    bool isClosed();
    bool canClose();
    void close();

    void halt();
    
    void loop();
    
    bool isMoving();

    void updateStatus();
    void updateLastCommunication();

    Status getStatus() const;
    ActualCommand getActualCommand() const;

    void validateConfiguration(const JsonObject &obj, JsonObject response);
    void storeConfiguration(JsonObject toBeStored,const char* schema);
    void getConfiguration(JsonObject obj);

    int getOpenSensorRaw(){ return OpenSensor.status(); };
    int getCloseSensorRaw(){ return CloseSensor.status(); };

    unsigned long lastTravelTime(){ return travelTime / 1000; }
    bool isAutoCloseEnable(){ return autoClose.enable; };
    unsigned long autoCloseRemaningTime() { return autoClose.remaningTime; };

private:
    void cycle();

    void checkTravelTimeOut();
    void checkAutoCloseTimeOut();
    void setOutput(outputDirection direction);
    void setOutputforGateBoard(outputDirection direction);
    void setOutputforDirectionalOutput(outputDirection direction);
    void setOutputforStartAndDirectionalOutput(outputDirection direction);
};

#endif