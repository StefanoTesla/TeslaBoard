#include <Arduino.h>
#include "PWMOutput.h"
#include "IOConfigStruct.h"


void PWMOutput::setup(IOConfigBase* config){
    if (config->getType() != 3) {
        Serial.println("Error: Configuration type of PWM not valid!");
        return;
    }

    PWMOutputConfig* cfg = static_cast<PWMOutputConfig*>(config);
    if(cfg->fastPWM){
        ledcSetup(cfg->ledChannel, 19531, 12);
        ledcAttachPin(cfg->pin,cfg->ledChannel);           
        pin = cfg->pin;
        min = 0;
        max = 4095;
        Serial.print("New 20kHz PWM setup at pin: ");
        Serial.print(pin);
    } else {
        ledcSetup(cfg->ledChannel, 5000, 12);
        ledcAttachPin(cfg->pin,cfg->ledChannel); 
            pin = cfg->pin;
            min = 0;
            max = 4095;
            Serial.print("New 5kHz PWM setup at pin: ");
            Serial.println(pin);
    }

}

void PWMOutput::jsonSetup(JsonObject setup, bool HS){

    channel = -1;

    if(HS){
        channel = chMgr->getFastChannel(true);
    } else {
        channel = chMgr->getFastChannel();
    }

    if(channel<0){
        Serial.println("Unable to retrive a free ledChannel");
        return;
    }
    uint32_t freq;
    if(HS){ 
        freq = 19531;
    } else {
        freq = 5000;
    }

    pin = setup["pin"].as<unsigned int>();
    min = 0;
    max = 4095;
    ledcSetup(pin, freq, 12);
    ledcAttachPin(pin,channel); 

}


void PWMOutput::getConfiguration(JsonObject cfg){
    cfg["pin"] = pin;
}

bool PWMOutput::pinUnusable(int pin){
    if(pin == 1 or pin == 3 or (pin >=6 and pin <=11) or pin == 20 or pin == 24 or (pin >=28 and pin <= 31)){
        return true;
    }
    
    if(pin > 33){
        return true;
    }
  return false;
}


int PWMOutput::validateJsonCfg(JsonObject json){

    if(!json["pin"].is<unsigned int>()){
      return -1;
    } else {
        if(pinUnusable(json["pin"].as<unsigned int>())){
            return -10;
        }
    }


    return 1;
}

void PWMOutput::copyJsonCfg(JsonObject src,JsonObject dest){
    dest["pin"] = src["pin"];
}

int PWMOutput::write(int _value) {
    if(_value >= min && _value <= max){
        ledcWrite(pin,_value);
        return 1;
    }
    
    return 0;

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

    