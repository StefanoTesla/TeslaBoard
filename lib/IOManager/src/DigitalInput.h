#ifndef DIGITAL_INPUT_H
#define DIGITAL_INPUT_H

#include "IOBase.h"

class DigitalInput : public IOBase {

private:
    bool value;
    bool reInput;
    bool feInput;
    unsigned long ackMillis;

public:
    int invert;
    unsigned long dOn;
    unsigned long dOff;

    DigitalInput();
    void setup(IOConfigBase* config);
    int readPin() override;
    int write(int _value) override;
    int status();
    int getType() override;

};

#endif