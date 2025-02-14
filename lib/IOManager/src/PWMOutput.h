#ifndef PWM_OUTPUT_H
#define PWM_OUTPUT_H

#include "IOBase.h"

class PWMOutput : public IOBase {

private:
    int channel;
    int value;

public:
    PWMOutput();
    void setup(IOConfigBase* config);
    int readPin() override;
    int write(int valore) override;
    int status();
    unsigned int getChannel();
    int getType() override;
};

#endif