#include <Arduino.h>
#include "PWMOutput.h"

PWMOutput::PWMOutput(){
    
}

void PWMOutput::setup(int _pin, int _channel){
    pin = _pin;
    channel = _channel;
    ledcAttachPin(pin,channel);
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

    