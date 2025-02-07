#ifndef DIGITAL_INPUT_H
#define DIGITAL_INPUT_H

#include "IOBase.h"

class DigitalInput : public IOBase {

private:
    bool value;
    bool invert;
    unsigned long ton;
    unsigned long toff;
    bool reInput;
    bool feInput;
    unsigned long ackMillis;
    int type = 1;

public:
    DigitalInput();
    void setup(int _pin, bool _invert = false, unsigned long _ton = 0, unsigned long _toff = 0);
    int readPin() override;
    int write(int _value) override;
    int status();
    bool getInvert();
    unsigned long getTOn();
    unsigned long getTOff();
    void setInvert(bool _invert);
    void setTOn(unsigned long _ton);
    void setTOff(unsigned long _toff);
    int getType() override;

};

#endif