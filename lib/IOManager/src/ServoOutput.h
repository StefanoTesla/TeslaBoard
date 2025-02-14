#ifndef SERVO_OUTPUT_H
#define SERVO_OUTPUT_H

#include "IOBase.h"

class ServoOutput : public IOBase {

private:
    unsigned int channel;
    int value;

    bool invert;
    int cycle;
    bool moving;
    bool overridePosition;

    struct MoveToSlowlyStruct{
        unsigned int destination;
        unsigned long intervall;
        bool increment;
        int nextDeg;
        unsigned long actualMillis;
    };

    MoveToSlowlyStruct MoveToSlowly;

    void servoHandler();

public:
    unsigned int maxDeg;
    unsigned int closeDeg;
    unsigned int openDeg;
    unsigned int movingTime;

    ServoOutput();
    void setup(IOConfigBase* config);
    int readPin() override;
    int write(int _valore) override;
    bool goToSlowly(int _angle=0, bool overridePosition = false);
    int status();
    unsigned int getChannel();
    int getType() override;
    int setServoAngle(int _angle);
    bool isClose();
    bool isOpen();
    bool isMoving();
    void loop();
};

#endif