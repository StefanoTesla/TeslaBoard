#include <Arduino.h>
#include "IOStruct.h"
#include "DigitalInput.h"

DigitalInput::DigitalInput(){
}

void DigitalInput::setup(IOConfigBase* config) {
    // Verifica che il tipo del config sia quello corretto (DigitalInputConfig)
    if (config->getType() == 1) {  // 1 è il tipo di DigitalInputConfig
        DigitalInputConfig* cfg = static_cast<DigitalInputConfig*>(config);

        pin = cfg->pin;
        invert = cfg->invert;
        ton = cfg->ton;
        toff = cfg->toff;
        
        pinMode(pin, INPUT);
    } else {
        Serial.println("Errore: tipo di configurazione non valido!");
    }
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
