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
    bool overridePosition = false;
    int channel = -1;
    unsigned long movingTime = 0;

    PWMManager* chMgr;

    struct MoveToSlowlyStruct{
        unsigned int destination = 0;
        unsigned int destInDuty = 0;
        unsigned int nextStep = 0;
        unsigned long intervall = 0;
        bool increment = false;
        unsigned long actualMillis = 0;
    };

    MoveToSlowlyStruct moveTo;

    void servoHandler();

public:
    int currentAngle;

    ServoOutput(PWMManager* channelManager) : chMgr(channelManager) {}
    void setup(IOConfigBase* config);
    void jsonSetup(JsonObject setup);
    void getConfiguration(JsonObject cfg);
    int validateJsonCfg(JsonObject obj);
    bool pinUnusable(int pin);
    void copyJsonCfg(JsonObject obj,JsonObject dest);


    int write(int _angle) override;
//    bool goToSlowly(int _percentage=0, bool overridePosition = true);
    int status();
    unsigned int getChannel(){return channel;}
    int getType() override;
    int readPosition();
    bool isMoving(){ return positioning; };
    void halt();
    void loop();
    void setMax(int _value);
    void goTo(int _percentage,bool direct = false,bool _oPos = false); //used only for switch, software decide to perform a direct or slow moviment
    bool isReferenced();
    void setMovingTime(unsigned int _time);
    unsigned int getMovingTime(){ return movingTime / 1000;}

private:
    void convertPosInduty(int position);
    void handleMovement();
    int readPin() override;
};

#endif