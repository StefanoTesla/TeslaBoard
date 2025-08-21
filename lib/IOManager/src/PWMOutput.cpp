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
        if(ledcAttach(cfg->pin,19531,12)){
            
            pin = cfg->pin;
            min = 0;
            max = 4095;
            Serial.print("New 20kHz PWM setup at pin: ");
            Serial.print(pin);

        } else {
            Serial.println("Error: Unable to assign PWM channel!");
        }
    } else {
        if(ledcAttach(cfg->pin,5000,12)){
            pin = cfg->pin;
            min = 0;
            max = 4095;
            Serial.print("New 5kHz PWM setup at pin: ");
            Serial.print(pin);
        } else {
            Serial.println("Error: Unable to assign PWM channel!");
        }
    }

}

int PWMOutput::write(int _value) {
    Serial.println(_value);
    if(_value >= min && _value <= max){
        if(ledcWrite(pin,_value)){
            Serial.println("ulalah");
        } else {
            Serial.println("uff");
        }
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

    