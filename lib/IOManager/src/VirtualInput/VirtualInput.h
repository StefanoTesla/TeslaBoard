#ifndef VIRTUAL_INPUT_H
#define VIRTUAL_INPUT_H

#include "IOBase.h"

class VirtualInput : public IOBase {

private:
    int value;
    int multiplier;
    int defaultVal;

public:
    
    VirtualInput();
    bool jsonSetup(JsonObjectConst obj,bool HS = false) override;
    static bool pinUnusable(int pin);
    static int validateJsonCfg(JsonObject obj);
    static void copyJsonCfg(JsonObject obj,JsonObject dest);
    void getConfiguration(JsonObject obj) override;
    void setInvert(bool newInvert) override;

    int write(int valore) override;
    int status();
    int getType() override;
    
private:
    int readPin() override;
};

#endif