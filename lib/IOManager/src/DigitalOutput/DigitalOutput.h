#ifndef DIGITAL_OUTPUT_H
#define DIGITAL_OUTPUT_H

#include "IOBase.h"

class DigitalOutput : public IOBase {

private:
    bool value;

public:
    bool invert = false;
    
    DigitalOutput();
    void setup(IOConfigBase* config);
    void jsonSetup(JsonObject obj);
    bool pinUnusable(int pin);
    int validateJsonCfg(JsonObject obj);
    void copyJsonCfg(JsonObject obj,JsonObject dest);
    void getConfiguration(JsonObject obj);
    int readPin() override;
    int write(int valore) override;
    int status();
    int getType() override;
};

#endif