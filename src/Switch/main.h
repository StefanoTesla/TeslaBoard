#ifndef SWITCH_MAIN
#define SWITCH_MAIN

#include "config.h"

void updateSwitchState(){
    //used only for run servo positioning and input delay time
    for(int i=0;i<Switch.config.configuredSwitch;i++){
        if(SwitchObjects[i] == nullptr){
            continue;
        }
        if(SwitchObjects[i]->getType() == SwTypeDInput || SwitchObjects[i]->getType() == SwTypeServo){
            SwitchObjects[i]->status();
        }
    }
}


void SwitchLoop(){
    updateSwitchState();


    if(Switch.config.save.execute){
        Switch.config.save.execute = false;
        saveSwitchConfig();
    }
}


#include "webserver.h"
#include "alpaca.h"

void switchRequestHandler(){
  switchWebServer();
  switchAlpacaManage();
  switchAlpacaDevice();
}
#endif