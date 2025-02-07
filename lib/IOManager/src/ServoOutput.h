#ifndef SERVO_OUTPUT_H
#define SERVO_OUTPUT_H

#include "IOBase.h"

class ServoOutput : public IOBase {

private:
    unsigned int channel;
    int value;
    int maxDeg;
    int closeDeg;
    int openDeg;
    bool invert;
    int cycle;
    bool moving;

public:
    ServoOutput();
    void setup(int _pin, int _channel);
    int readPin() override;
    int write(int _valore) override;
    int status();
    unsigned int getChannel();
    int getType() override;
    int setServoAngle(int _angle);
    void slowMove(int _angle);
    bool isClose();
    bool isOpen();
    bool isMoving();
    void loop();
};

#endif