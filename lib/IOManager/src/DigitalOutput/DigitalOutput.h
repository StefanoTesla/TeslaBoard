#ifndef DIGITAL_OUTPUT_H
#define DIGITAL_OUTPUT_H

#include "IOBase.h"

class DigitalOutput : public IOBase {

private:
    bool value;

public:
    bool invert = false;
    
    DigitalOutput();
    bool jsonSetup(JsonObjectConst obj,bool HS = false) override;
    static bool pinUnusable(int pin);
    static int validateJsonCfg(JsonObject obj);
    static void copyJsonCfg(JsonObject obj,JsonObject dest);
    void getConfiguration(JsonObject obj) override;

    int write(int valore) override;
    int status();
    int getType() override;
    
private:
    int readPin() override;
};

#endif