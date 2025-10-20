#ifndef SERVO_OUTPUT_H
#define SERVO_OUTPUT_H

#include "IOBase.h"
#include <PWMManager.h>
#include <ArduinoJson.h>

class ServoOutput : public IOBase {

private:
    int value;
    int cycle = 0;
    bool positioning = false;
    bool overridePosition = true;
    int channel = -1;

    PWMManager* chMgr;

    struct MoveToSlowlyStruct{
        unsigned long startTime = 0;
        unsigned long endTime = 0;
        unsigned int destination = 0;
        unsigned long intervall = 0;
        bool increment = false;
        int nextDeg = 0;
        unsigned long actualMillis = 0;
    };

    MoveToSlowlyStruct MoveToSlowly;

    void servoHandler();

public:
    unsigned int closeDeg = 0;
    unsigned int openDeg = 0;
    unsigned long movingTime = 0;
    int currentAngle;

    ServoOutput(PWMManager* channelManager) : chMgr(channelManager) {}
    void setup(IOConfigBase* config);
    void jsonSetup(JsonObject setup);
    void getConfiguration(JsonObject cfg);
    int validateJsonCfg(JsonObject obj);
    void copyJsonCfg(JsonObject obj,JsonObject dest);
    bool pinUnusable(int pin);
    int readPin() override;
    int write(int _angle) override;
    bool goToSlowly(int _percentage=0, bool overridePosition = true);
    int status();
    unsigned int getChannel(){return channel;}
    int getType() override;
    int readAngle();
    bool isMoving(){ return isMoving; };
    void halt();
    void loop();
    void setMax(int _value);
    void goTo(int _percentage,bool slowPermitted); //used only for switch, software decide to perform a direct or slow moviment
    bool isReferenced();

private:
    void handleMovement();
};

#endif