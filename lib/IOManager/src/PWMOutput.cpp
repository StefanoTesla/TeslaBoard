#include <Arduino.h>
#include "PWMOutput.h"

PWMOutput::PWMOutput(){
    
}

void PWMOutput::setup(int _pin, int _channel){
    pin = _pin;
    channel = _channel;
    ledcAttachPin(pin,channel);
}

int PWMOutput::write(int _valore) {
    if(_valore > 0 && _valore < 4096){
        ledcWrite(channel,_valore);
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

    