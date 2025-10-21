#ifndef PWM_OUTPUT_H
#define PWM_OUTPUT_H

#include "IOBase.h"
#include <PWMManager.h>

class PWMOutput : public IOBase {

public:
    PWMOutput(PWMManager* channelManager) : chMgr(channelManager) {}
    unsigned int currentDuty;
    void setup(IOConfigBase* config);
    void jsonSetup(JsonObject setup,bool HS = false);
    void getConfiguration(JsonObject cfg);
    int validateJsonCfg(JsonObject obj);
    void copyJsonCfg(JsonObject obj,JsonObject dest);
    
    int write(int value) override;
    int status();
    unsigned int getMax();
    int getChannel(){return channel;}
    int getType() override;
    bool pinUnusable(int pin);

private:
    int value;
    int channel=-1;
    PWMManager* chMgr;
    int readPin() override;
};

#endif