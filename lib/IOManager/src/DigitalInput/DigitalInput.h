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
    bool jsonSetup(JsonObjectConst obj,bool HS = false) override;
    static bool pinUnusable(int pin);
    static int validateJsonCfg(JsonObject obj);
    static void copyJsonCfg(JsonObject obj,JsonObject dest);
    void getConfiguration(JsonObject obj) override;
    int getType() override;
    int write(int _value) override;
    int status();
    void loop() override;
    


private:
    int readPin() override;
};

#endif