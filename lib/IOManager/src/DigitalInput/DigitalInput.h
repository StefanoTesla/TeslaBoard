#ifndef DIGITAL_INPUT_H
#define DIGITAL_INPUT_H

#include "IOBase.h"

class DigitalInput : public IOBase {

private:
    int value = 0;
    bool reInput = false;
    bool feInput = false;
    unsigned long ackMillis = 0;

public:
    bool invert = false;
    unsigned long dOn = 0;
    unsigned long dOff = 0;

    DigitalInput();
    void setup(IOConfigBase* config);
    void jsonSetup(JsonObjectConst obj);
    bool pinUnusable(int pin);
    int validateJsonCfg(JsonObject obj);
    void copyJsonCfg(JsonObject obj,JsonObject dest);
    int write(int _value) override;
    int status();
    int getType() override;
    void getConfiguration(JsonObject obj);

private:
    int readPin() override;
};

#endif