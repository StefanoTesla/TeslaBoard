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


void DigitalOutput::jsonSetup(JsonObjectConst obj){
    pin = obj["pin"].as<unsigned int>();
    invert = obj["invert"].as<unsigned int>();
    pinMode(pin, OUTPUT);
    Serial.print("New DO setup at pin: ");
    Serial.println(pin);
    write(0);
}

bool DigitalOutput::pinUnusable(int pin){
    if(pin == 1 or pin == 3 or (pin >=6 and pin <=11) or pin == 20 or pin == 24 or (pin >=28 and pin <= 31)){
        return true;
    }
    
    if(pin > 33){
        return true;
    }
  return false;
}

int DigitalOutput::validateJsonCfg(JsonObject json){

    if(!json["pin"].is<unsigned int>()){
      return -1;
    } else {
        if(pinUnusable(json["pin"].as<unsigned int>())){
            return -10;
        }
    }
    if(!json["invert"].is<bool>()){
        return -4;
    }

    return 1;
}

/**
 * @brief This funcition copy only the essential json key.
 * 
 * This function copy the json configuration keys requested by the DigitalOutput
 * This function copy the src["pin"] and src["invert"] to the dest object
 * 
 * @param src where key are present
 * @param dest where keys are copied
 * 
 * @return nothing
 */
void DigitalOutput::copyJsonCfg(JsonObject src,JsonObject dest){
    dest["pin"] = src["pin"];
    dest["invert"] = src["invert"];

}

void DigitalOutput::getConfiguration(JsonObject obj){
    obj["pin"] = pin;
    obj["invert"] = invert;
}


/**
 * @brief Write the value to the gpio.
 * 
 * Questa funzione utilizza `digitaldigitalWrite()` per scrivere il valore
 * di un pin, se configurato, lo inverte
 * 
 * @param _value value to be written to the GPIO
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

    