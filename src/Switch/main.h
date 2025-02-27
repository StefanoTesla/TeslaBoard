#ifndef SWITCH_MAIN
#define SWITCH_MAIN

#include "config.h"

void updateSwitchState(){
    for(int i=0;i<Switch.config.configuredSwitch;i++){
        if(SwitchObjects[i] == nullptr){ continue; }
        SwitchObjects[i]->status();
    }
}


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