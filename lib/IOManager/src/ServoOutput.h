#ifndef SERVO_OUTPUT_H
#define SERVO_OUTPUT_H

#include "IOBase.h"
#include "IOConfigStruct.h"

class ServoOutput : public IOBase {

private:
    unsigned int channel;
    int value;
    bool invert;
    int cycle;
    bool moving;
    bool overridePosition;

    struct MoveToSlowlyStruct{
        unsigned long startTime;
        unsigned long endTime;
        unsigned int destination;
        unsigned long intervall;
        bool increment;
        int nextDeg;
        unsigned long actualMillis;
    };

    MoveToSlowlyStruct MoveToSlowly;

    void servoHandler();

public:
    unsigned int closeDeg;
    unsigned int openDeg;
    unsigned long movingTime;
    int currentAngle;

    ServoOutput();
    void setup(IOConfigBase* config);
    int readPin() override;
    int write(int _angle) override;
    bool goToSlowly(int _angle=0, bool overridePosition = true);
    int status();
    unsigned int getChannel();
    int getType() override;
    int setServoAngle(int _angle);
    int readAngle();
    bool isClose();
    bool isOpen();
    bool isMoving();
    void loop();
    void setMax(int _value);
};

#endif