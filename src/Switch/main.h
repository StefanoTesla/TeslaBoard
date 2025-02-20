#ifndef SWITCH_MAIN
#define SWITCH_MAIN

#include "config.h"

void updateSwitchState(){
    for(int i=0;i<Switch.config.configuredSwitch;i++){
        if(SwitchObjects[i] == nullptr){ continue; }
        SwitchObjects[i]->status();
    }
}

/*
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
*/

void SwitchLoop(){
    updateSwitchState();
    //setSwitchValue();


    if(Switch.config.save.execute){
        Switch.config.save.execute = false;
        saveSwitchConfig();
    }
}


#include "webserver.h"
#include "alpacaManage.h"
//#include "alpacaDevice.h"

void switchRequestHandler(){
  switchWebServer();
  //switchAlpacaManage();
  //switchAlpacaDevice();
}
#endif