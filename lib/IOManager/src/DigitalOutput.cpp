#include <Arduino.h>
#include "DigitalOutput.h"
#include "IOConfigStruct.h"

DigitalOutput::DigitalOutput(){
    
}

void DigitalOutput::setup(IOConfigBase* config){
    if (config->getType() == 2) { 
        DigitalOutputConfig* cfg = static_cast<DigitalOutputConfig*>(config);

        pin = cfg->pin;
        invert = cfg->invert;
        min = 0;
        max = 1;
        pinMode(pin, OUTPUT);
        Serial.print("New DO setup at pin: ");
        Serial.println(pin);
        write(0);
    } else {
        Serial.println("Errore: DO tipo di configurazione non valido!");
    }
}

/**
 * @brief Scrive il valore digitale nel pin.
 * 
 * Questa funzione utilizza `digitaldigitalWrite()` per scrivere il valore
 * di un pin, se configurato, lo inverte
 * 
 * @return int 1= operazione completata.
 */
int DigitalOutput::write(int _value) {
    //never trust what user can write in _value
    if(invert){
        if(_value == 0){
            _value =1;
        } else {
            _value = 0;
        }
    }
    digitalWrite(pin,_value);
    return 1;
}

int DigitalOutput::readPin() {
    return digitalRead(pin);
}

int DigitalOutput::status(){
    
    value = invert ? !readPin() : readPin();
    return value;
}

int DigitalOutput::getType(){
    return 2;
}

    