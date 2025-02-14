#include <Arduino.h>
#include "DigitalOutput.h"
#include "IOConfigStruct.h"

DigitalOutput::DigitalOutput(){
    
}

void DigitalOutput::setup(IOConfigBase* config){
    if (config->getType() == 3) { 
        DigitalOutputConfig* cfg = static_cast<DigitalOutputConfig*>(config);

        pin = cfg->pin;
        
        pinMode(pin, OUTPUT);
    } else {
        Serial.println("Errore: tipo di configurazione non valido!");
    }
}


int DigitalOutput::write(int _value) {
    digitalWrite(pin,_value);
    return 1;
}

int DigitalOutput::readPin() {
    return digitalRead(pin);
}

int DigitalOutput::status(){
    value = readPin();
    return value;
}

int DigitalOutput::getType(){
    return 3;
}

    