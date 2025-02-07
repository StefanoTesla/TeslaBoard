#include <Arduino.h>
#include "DigitalInput.h"

DigitalInput::DigitalInput(){
}

void DigitalInput::setup(int _pin, bool _invert, unsigned long _ton, unsigned long _toff){
    pin = _pin;
    invert = _invert;
    ton = _ton;
    toff = _toff;
    pinMode(_pin, INPUT);
}


int DigitalInput::write(int _value) {
    Serial.println("Errore: Impossibile scrivere su un ingresso digitale.");
    return -1;
}

int DigitalInput::readPin() {
    return invert ? digitalRead(pin) : digitalRead(pin);
}

int DigitalInput::status(){
    
    /* ON CASE */
    if(readPin()){
        if(!value){
            if(!reInput){   /* get rising edge*/
                feInput = false;
                reInput = true;
                ackMillis = millis();
            } else {
                if(millis() - ackMillis > ton){ /*wait ton time*/
                    value = true;
                }
            }
        }
    } else {
        if(value){
            if(!feInput){   /* get falling edge*/
                reInput = false;
                feInput = true;
                ackMillis = millis();
            } else {
                if(millis() - ackMillis > toff){ /*wait toff time*/
                    value = false;
                }
            }
        }
    }

    return value;
}

bool DigitalInput::getInvert(){
    return invert;
}

unsigned long DigitalInput::getTOn(){
    return ton;
}

unsigned long DigitalInput::getTOff(){
    return toff;
}

void DigitalInput::setInvert(bool _invert){
    invert=_invert;
}

void DigitalInput::setTOn(unsigned long _ton){
    ton = _ton;
}

void DigitalInput::setTOff(unsigned long _toff){
    toff = _toff;
}

int DigitalInput::getType(){
    return 1;
}
