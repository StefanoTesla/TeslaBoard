#ifndef LOOP
#define LOOP

void main_loop(){

    saveConfig();
    #ifdef DOME 
    domehandlerloop();
    #endif

    #ifdef COVERC
    coverCLoop();
    #endif

    #ifdef SWITCH
    switchLoop();
    #endif

    #ifdef LIDAR
    lidarLoop();
    #endif
    
    ElegantOTA.loop();
    unsigned long currentMillis = millis();

    //get esp32 uptime
    if (currentMillis - Global.esp32.upTime.previousMillis > Global.upTimeInterval){
      Global.esp32.upTime.minutes +=1;
      Global.esp32.upTime.previousMillis = currentMillis;
    }

    //get wifi uptime and handle reconnection
    if(WiFi.status() == WL_CONNECTED ){
        Global.wifi.reconnection.waitToReconnect = false;
        if (currentMillis - Global.wifi.upTime.previousMillis > Global.upTimeInterval){
            Global.wifi.upTime.minutes +=1;
            Global.wifi.upTime.previousMillis = currentMillis;
        }
    } else {
        Global.wifi.upTime.previousMillis = currentMillis;
        if (Global.wifi.reconnection.waitToReconnect == false){
            Global.wifi.reconnection.waitToReconnect = true;
            Global.wifi.reconnection.lastMillis = currentMillis;
        }
        if (currentMillis - Global.wifi.reconnection.lastMillis >= Global.wifi.reconnection.intervall) {
            Global.wifi.reconnection.lastMillis = currentMillis;
            Serial.println("wifi lost, reconnection...");
            WiFi.disconnect();
            WiFi.reconnect();
            Global.wifi.upTime.minutes = 0;
        }
    }

}





#endif