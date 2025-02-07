#include <Arduino.h>
#include "DigitalOutput.h"

DigitalOutput::DigitalOutput(){
    
}

void DigitalOutput::setup(int _pin){
    pin = _pin;
    pinMode(_pin, OUTPUT);
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

    