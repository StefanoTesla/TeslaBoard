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


void DigitalInput::jsonSetup(JsonObjectConst obj){
    pin = obj["pin"].as<unsigned int>();
    invert = obj["invert"].as<unsigned int>();
    dOn = obj["dOn"].as<unsigned int>();
    dOff = obj["dOff"].as<unsigned int>();
    min = 0;
    max = 1;
    pinMode(pin, INPUT);
    Serial.print("New DI setup at pin: ");
    Serial.println(pin);
}

//
// return true if you can't use this pin
bool DigitalInput::pinUnusable(int pin){
    if(pin == 1 or pin == 3 or (pin >=6 and pin <=11) or pin == 20 or pin == 24 or (pin >=28 and pin <= 31) or pin == 37 or pin == 38 or pin > 39){
        return true;
    }
    
    if(pin==0 or pin==12){
        return true;
    }
  return false;
}

int DigitalInput::validateJsonCfg(JsonObject json){

    Serial.println("DIGITAL INPUT VALIDATION");
    serializeJson(json,Serial);
    if(!json["pin"].is<unsigned int>()){
      return -1;
    } else {
        if(pinUnusable(json["pin"].as<unsigned int>())){
            return -10;
        }
    }
    if(!json["dOn"].is<unsigned int>()){
        return -2;
    }
    if(!json["dOff"].is<unsigned int>()){
        return -3;
    }
    if(!json["invert"].is<bool>()){
        return -4;
    }

    return 1;
}


void DigitalInput::copyJsonCfg(JsonObject obj,JsonObject dest){
    dest["pin"] = obj["pin"];
    dest["invert"] = obj["invert"];
    dest["dOn"] = obj["dOn"];
    dest["dOff"] = obj["dOff"];

}

void DigitalInput::getConfiguration(JsonObject obj){
    obj["pin"] = pin;
    obj["invert"] = invert;
    obj["dOn"] = dOn;
    obj["dOff"] = dOff;

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
                    value = 1;
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
                    value = 0;
                }
            }
        }
    }

    return value;
}


int DigitalInput::getType(){
    return 1;
}
