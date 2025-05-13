#ifndef BOARD_MAIN
#define BOARN_MAIN
#include "configuration.h"


void startupTask(){


    initBoardConfig();

    #ifdef DOME
        initDomeConfig();
    #endif

    #ifdef COVER_CALIBRATOR
        initCoverCConfig();
    #endif

    #ifdef SWITCH
        initSwitchConfig();
    #endif

    printLEDChannelStatus();

}



void boardLoop(){
    if(Global.config.save.execute){
        saveBoardConfig();
        Global.config.save.execute = false;
    }

    if(Global.actualMillis - Global.pulse.second.oldMillis >= 1000){
        Global.pulse.second.pulse = true;
        Global.pulse.second.oldMillis = Global.actualMillis;
    } else { Global.pulse.second.pulse = false; }
    if(Global.actualMillis - Global.pulse.minute.oldMillis >= 60000){
        Global.pulse.minute.pulse = true;
        Global.pulse.minute.oldMillis = Global.actualMillis;
    } else { Global.pulse.minute.pulse = false; }

    
    
    if(Global.pulse.minute.pulse){
        //esp32 uptime
        Global.config.esp32.upTime.minutes +=1;

        //wifi uptime
        if(WiFi.status() == WL_CONNECTED){
            Global.config.wifi.upTime.minutes +=1;
        }
    }

    //wifi reconnection
    if(WiFi.status() == WL_CONNECTED ){
        Global.config.wifi.reconnection.waitToReconnect = false;
        Global.config.wifi.reconnection.delaySeconds = 0;
    } else {
         Global.config.wifi.upTime.minutes = 0;
        if(!Global.config.wifi.reconnection.waitToReconnect){
            Serial.println("wifi lost, wait interval...");
            Global.config.wifi.reconnection.waitToReconnect = true;
        } else {
            //WiFi.disconnect();
            if(Global.pulse.second.pulse){ Global.config.wifi.reconnection.delaySeconds += 1;}
            if(Global.config.wifi.reconnection.delaySeconds > Global.config.wifi.reconnection.intervall){
                Serial.println("wifi reconnection...");
                Global.config.wifi.reconnection.delaySeconds = 0;
                Global.config.wifi.upTime.minutes = 0;
                WiFi.reconnect();
            
            }
        }

    }

    if (Global.config.reboot.rebootRequest){
        if(Global.config.reboot.lastMillis == 0){
            Global.config.reboot.lastMillis = Global.actualMillis;
        } else {
            if((Global.actualMillis - Global.config.reboot.lastMillis) > 1000){
                ESP.restart();
            }
        }  
    }

    ElegantOTA.loop();
}

#endif