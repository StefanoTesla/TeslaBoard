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
        ledcSetup(cfg->ledChannel, 19531, 12);
        ledcAttachPin(cfg->pin,cfg->ledChannel);           
        pin = cfg->pin;
        min = 0;
        max = 4095;
        Serial.print("New 20kHz PWM setup at pin: ");
        Serial.print(pin);
    } else {
        ledcSetup(cfg->ledChannel, 5000, 12);
        ledcAttachPin(cfg->pin,cfg->ledChannel); 
            pin = cfg->pin;
            min = 0;
            max = 4095;
            Serial.print("New 5kHz PWM setup at pin: ");
            Serial.println(pin);
    }

}

int PWMOutput::write(int _value) {
    if(_value >= min && _value <= max){
        ledcWrite(pin,_value);
        return 1;
    }
    
    return 0;

}

unsigned int PWMOutput::getMax() {

    return max;

}

int PWMOutput::readPin() {
    /* esp hardware return 4096 when I wrote 4095 bah.. */
    currentDuty = ledcRead(pin);
    if(currentDuty > max){
        return max;
    }
    return currentDuty;
}

int PWMOutput::status(){
    return readPin();
}


int PWMOutput::getType(){
    return 3;
}

    