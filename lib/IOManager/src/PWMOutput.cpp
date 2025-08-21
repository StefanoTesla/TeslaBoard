#include <Arduino.h>
#include "PWMOutput.h"
#include "IOConfigStruct.h"

PWMOutput::PWMOutput(){
    
}

void PWMOutput::setup(IOConfigBase* config){
    if (config->getType() != 3) {
        Serial.println("Error: Configuration type of PWM not valid!");
        return;
    }

    PWMOutputConfig* cfg = static_cast<PWMOutputConfig*>(config);
    if(cfg->fastPWM){
        if(ledcAttach(pin,19531,12)){
            
            pin = cfg->pin;
            min = 0;
            max = 4095;
            Serial.print("New fast PWM setup at pin: ");
            Serial.print(pin);
            Serial.print(" at channel: ");
            Serial.println(channel);
        } else {
            Serial.println("Error: Unable to assign PWM channel!");
        }
    } else {
        if(ledcAttach(pin,5000,12)){
            pin = cfg->pin;
            min = 0;
            max = 4095;
            Serial.print("New PWM setup at pin: ");
            Serial.print(pin);
            Serial.print(" at channel: ");
            Serial.println(channel);
        } else {
            Serial.println("Error: Unable to assign PWM channel!");
        }



    }

}

int PWMOutput::write(int _value) {

    if(_value >= min && _value <= max){
        ledcWrite(channel,_value);
        return 1;
    } else {
        return 0;
    }

}

unsigned int PWMOutput::getMax() {

    return max;

}

int PWMOutput::readPin() {
    /* esp hardware return 4096 when I wrote 4095 bah.. */
    currentDuty = ledcRead(channel);
    if(currentDuty > max){
        return max;
    }
    return currentDuty;
}

int PWMOutput::status(){
    return readPin();
}

unsigned int PWMOutput::getChannel(){
    return channel;
}

int PWMOutput::getType(){
    return 3;
}

    