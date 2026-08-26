#ifndef VIRTUAL_INPUT_H
#define VIRTUAL_INPUT_H

#include "IOBase.h"

class VirtualInput : public IOBase {

private:
    int value = 0;
    int defaultVal = 0;
    unsigned long lastRefresh = 0;
    unsigned int expiration = 0;

public:
    
    VirtualInput();
    bool jsonSetup(JsonObjectConst obj,bool HS = false) override;
    static bool pinUnusable(int pin);
    static int validateJsonCfg(JsonObject obj);
    static void copyJsonCfg(JsonObject obj,JsonObject dest);
    void getConfiguration(JsonObject obj) override;
    void setDefault(int value) override;
    void setExpiration(int value) override;
    int write(int valore) override;
    int status();
    int getType() override;
    void loop();
    
private:
    int readPin() override;
};

#endif