#ifndef SHUTTER_H
#define SHUTTER_H

#include <Arduino.h>
#include "IOManager.h"
#include <ArduinoJson.h>
#include "esp_log.h"
#include <NVSManager.h>

class Shutter {
public:
    enum Status {
        Opened,
        Closed,
        Opening,
        Closing,
        Error
    };

    enum ShutterError {
        None,
        TOutOpening,
        TOutClosing,
        Halted
    };

    enum ActualCommand {
        Idle,
        Open,
        Close,
        Halt
    };


private:
    Status status = Error;
    ShutterError error = None;
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

    struct autoCloseConfig{
        bool enable = false;
        unsigned long waitingTime = 30; //[min] during the setup is multiplied by 60000 to conver minutes to ms
        unsigned long lastCommunication; //[ms] the last comunication millis from api or alpaca
    };

    autoCloseConfig autoClose;

    unsigned long ackTimeout = 0;

    struct logging{
        bool openState;
        bool closeState;
        bool startOutput;
        bool haltOutput;
        ActualCommand cmd;
        CycleStep cycle;
        Status status;
    };


    struct logger{
        logging actual;
        logging previous;
    };

    logger log;

    unsigned long startTravelMillis;
    unsigned long travelTime;
    unsigned long travelTOUT = 20000;

    bool retry = false; //used only for gate controller board

    bool moduleEnable = false;
    
    JsonDocument tmpCfg;


public:
    Shutter() = default;

    void begin(const JsonDocument& shutter);
    bool isEnable(){ return moduleEnable; }

    bool isOpen();
    bool canOpen();
    void open();

    bool isClosed();
    bool canClose();
    void close();

    int getError() {return error;};

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

    int getOpenSensorRaw(){ return OpenSensor.status(); }
    int getCloseSensorRaw(){ return CloseSensor.status(); }
    unsigned int getTravelTimeOut(){return travelTOUT / 1000;}
    void setTravelTimeOut(unsigned int time);
    unsigned long lastTravelTime(){ return travelTime; }
    bool isAutoCloseEnable(){ return autoClose.enable; };

    unsigned int getAutoCloseTimeMin(){return autoClose.waitingTime / 60000;}
    void setAutoCloseTimeMin(unsigned int minutes);


private:
    void cycle();

    void checkTravelTimeOut();
    void checkAutoCloseTimeOut();
    void setOutput(outputDirection direction);
    void setOutputforGateBoard(outputDirection direction);
    void setOutputforDirectionalOutput(outputDirection direction);
    void setOutputforStartAndDirectionalOutput(outputDirection direction);
    void debug();
};

#endif