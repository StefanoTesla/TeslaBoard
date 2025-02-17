#include <Arduino.h>
#include "PWMOutput.h"
#include "IOConfigStruct.h"

PWMOutput::PWMOutput(){
    
}

void PWMOutput::setup(IOConfigBase* config){
    if (config->getType() == 4) {  // 1 è il tipo di DigitalInputConfig
        PWMOutputConfig* cfg = static_cast<PWMOutputConfig*>(config);
        pin = cfg->pin;
        channel = cfg->channel;
        min = 0;
        max = 4095;
        ledcAttachPin(pin,channel);
        Serial.print("New PWM setup at pin: ");
        Serial.print(pin);
        Serial.print(" at channel: ");
        Serial.println(channel);
    } else {
        Serial.println("Errore: PWM tipo di configurazione non valido!");
    }
}

int PWMOutput::write(int _value) {
    Serial.print("Canale: ");
    Serial.print(channel);
    Serial.print(" valore: ");
    Serial.print(_value);
    
    if(_value >= min && _value <= max){
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

    