#ifndef PWM_OUTPUT_H
#define PWM_OUTPUT_H

#include "IOBase.h"
#include <PWMManager.h>

class PWMOutput : public IOBase {

public:
    PWMOutput(PWMManager* channelManager) : chMgr(channelManager) {}
    unsigned int currentDuty;
    bool jsonSetup(JsonObjectConst setup,bool HS = false) override;
    void getConfiguration(JsonObject cfg) override;
    static int validateJsonCfg(JsonObject obj);
    static bool pinUnusable(int pin);
    static void copyJsonCfg(JsonObject obj,JsonObject dest);
    int getType() override;

    int write(int value) override;
    int status();
    unsigned int getMax();
    int getChannel(){return channel;}
    
private:
    int value;
    int channel=-1;
    PWMManager* chMgr;
    int readPin() override;
};

#endif