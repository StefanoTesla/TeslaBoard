#include <Arduino.h>
#include "IOConfigStruct.h"
#include "DigitalInput.h"

DigitalInput::DigitalInput(){
}

void DigitalInput::setup(IOConfigBase* config){
    if (config->getType() == 1) {  // 1 è il tipo di DigitalInputConfig
        DigitalInputConfig* cfg = static_cast<DigitalInputConfig*>(config);
        pin = cfg->pin;
        invert = cfg->invert;
        dOn = cfg->dOn;
        dOff = cfg->dOff;
        min = 0;
        max = 1;
        pinMode(pin, INPUT);
        Serial.print("New DI setup at pin: ");
        Serial.println(pin);
    } else {
        Serial.println("Errore: DI tipo di configurazione non valido!");
    }
}


int DigitalInput::write(int _value) {
    Serial.println("Errore: Impossibile scrivere su un ingresso digitale.");
    return -1;
}

int DigitalInput::readPin() {
    return invert ? !digitalRead(pin) : digitalRead(pin);
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
                if(millis() - ackMillis > dOn){ /*wait ton time*/
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
                if(millis() - ackMillis > dOff){ /*wait toff time*/
                    value = false;
                }
            }
        }
    }

    return value;
}


int DigitalInput::getType(){
    return 1;
}
