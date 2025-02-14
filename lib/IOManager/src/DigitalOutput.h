#ifndef DIGITAL_OUTPUT_H
#define DIGITAL_OUTPUT_H

#include "IOBase.h"

class DigitalOutput : public IOBase {

private:
    int pin;
    bool value;

public:
    DigitalOutput();
    void setup(IOConfigBase* config);
    int readPin() override;
    int write(int valore) override;
    int status();
    int getType() override;
};

#endif