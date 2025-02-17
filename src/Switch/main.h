#ifndef SWITCH_MAIN
#define SWITCH_MAIN

#include "config.h"

void updateSwitchState(){
    for(int i=0;i<_MAX_SWITCH_ID_;i++){
        int value = 0;
        switch(Switch.data[i].property.type){

            case SwTypeNull:
                Switch.data[i].actualValue.boValue =false;
                Switch.data[i].actualValue.intValue = 0;
                break;
            case SwTypeDInput:
            case SwTypeDOutput:
                digitalRead(Switch.data[i].property.pin);
                Switch.data[i].actualValue.boValue = SwitchObjects[i]->status() ? true : false;
                Switch.data[i].actualValue.intValue = SwitchObjects[i]->status();
                break;
            case SwTypeAInput:
            case SwTypeAOutput:
                value = map(analogRead(Switch.data[i].property.pin),0,4095,Switch.data[i].property.minValue,Switch.data[i].property.maxValue);
                Switch.data[i].actualValue.boValue = value > Switch.data[i].property.minValue ? true : false;
                Switch.data[i].actualValue.intValue = value;
                break;
            case SwTypePWM:
                Switch.data[i].actualValue.intValue = map(ledcRead(Switch.data[i].property.pwmch),0,4096,Switch.data[i].property.minValue,Switch.data[i].property.maxValue);
                Switch.data[i].actualValue.boValue = Switch.data[i].actualValue.intValue > Switch.data[i].property.minValue ? true : false;
                break;
            case SwTypeServo:
                value  = ledcRead(Switch.data[i].property.pwmch);
                break;
            default:
                break;
        }

    }
    
}


void setSwitchValue(){
    for(int i=0;i<_MAX_SWITCH_ID_;i++){
        if(!Switch.data[i].command.execute){
            continue;
        }
        int value;
        int dutyMicros;
        int dutyValue;
        switch(Switch.data[i].property.type){

            case SwTypeNull:
                Switch.data[i].actualValue.intValue = 0;
                Switch.data[i].actualValue.boValue = false;
                break;
            case SwTypeDOutput:
                digitalWrite(Switch.data[i].property.pin,Switch.data[i].command.boValue);
                logMessageFormatted(Switches,lInfo,"DO Switch Command Executed on ID %d",i);
                break;
            case SwTypePWM:
                ledcWrite(Switch.data[i].property.pwmch,Switch.data[i].command.intValue);
                logMessageFormatted(Switches,lInfo,"PWM Switch Command Executed on ID %d with val: %d",i,Switch.data[i].command.intValue);
                break;
            case SwTypeServo:
                logMessageFormatted(Switches,lInfo,"Servo Switch Command Executed on ID %d with val: %d",i,Switch.data[i].command.intValue);
                ledcWrite(Switch.data[i].property.pwmch,dutyValue);
                break;
            default:
                logMessage(Switches,lErr,"Trying to command a non writable Switch");
                break;
        }
        
        Switch.data[i].command.execute = false;

    }

}


void SwitchLoop(){
    updateSwitchState();
    setSwitchValue();


    if(Switch.config.save.execute){
        Switch.config.save.execute = false;
        saveSwitchConfig();
    }
}


#include "webserver.h"
#include "alpacaManage.h"
#include "alpacaDevice.h"

void switchRequestHandler(){
  switchWebServer();
  switchAlpacaManage();
  switchAlpacaDevice();
}
#endif