#include <Arduino.h>
#include "TeslaWiFiAP.h"
#include <WiFi.h>
#undef LOG_TAG
#define LOG_TAG "WiFiAPMgr"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

void TeslaWiFiAP::loop(){

    while(1){

        switch (cycle)
        {
        case WAIT_FOR_A_COMMAND:
            
            if(NONE == cmd){ return;}

            if(START == cmd){
                LOGV("AP start request");
                if(status!= Starting || status!= Started){
                    cycle = STARTING_THE_AP;
                }
                cmd = NONE;
                break;           
            }

            if(STOP == cmd){
                LOGV("AP stop request");
                if(status != Closing || status!= Closed){
                    cycle = STOPPING_THE_AP;
                }
                cmd = NONE;
                break;
            }
            break;

        case STARTING_THE_AP:
            cmd == NONE;
            status = Starting;
            WiFi.enableAP(true);
            WiFi.softAP("TeslaBoard", "123456789");
            WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
            cycle = WAIT_AP_RUNNING;
            break;

        case WAIT_AP_RUNNING:
            while(WiFi.softAPIP() != IPAddress(192,168,4,1)){ return; }
            LOGV("AP got the ip, AP is running");
            status = Started;
            cycle = WAIT_FOR_A_COMMAND;
            break;

        case STOPPING_THE_AP:
            cmd == NONE;
            status = Closing;
            WiFi.softAPdisconnect(false);
            cycle = WAIT_AP_CLOSED;
            break;

        case WAIT_AP_CLOSED:
            LOGV("AP was disconnected");
            status = Closed;
            cycle = WAIT_FOR_A_COMMAND;
            break;
        
        default:
            break;
        }

    }

}


void TeslaWiFiAP::startAP(){
    cmd = START;
}
void TeslaWiFiAP::stopAP(){
    cmd = STOP;
}


TeslaWiFiAP::Status TeslaWiFiAP::getStatus(){
    return status;
}
