#ifndef DIGITAL_OUTPUT_H
#define DIGITAL_OUTPUT_H

#include "IOBase.h"

class DigitalOutput : public IOBase {

private:
    bool value;

public:
    int invert = 0;
    
    DigitalOutput();
    void setup(IOConfigBase* config);
    int readPin() override;
    int write(int valore) override;
    int status();
    int getType() override;
};

#endif