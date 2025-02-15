#include <Arduino.h>
#include "PWMOutput.h"
#include "IOConfigStruct.h"

PWMOutput::PWMOutput(){
    
}

void PWMOutput::setup(IOConfigBase* config){
    if (config->getType() == 1) {  // 1 è il tipo di DigitalInputConfig
        PWMOutputConfig* cfg = static_cast<PWMOutputConfig*>(config);
        pin = cfg->pin;
        channel = cfg->channel;
        ledcAttachPin(pin,channel);
    }
}

int PWMOutput::write(int _value) {
    if(_value > 0 && _value < 4096){
        ledcWrite(channel,_value);
        return 1;
    } else {
        return 0;
    }

}

int PWMOutput::readPin() {
    return ledcRead(channel);
}

int PWMOutput::status(){
    channel = readPin();
    return channel;
}

unsigned int PWMOutput::getChannel(){
    return channel;
}

int PWMOutput::getType(){
    return 4;
}

    