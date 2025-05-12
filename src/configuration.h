#ifndef CONFIGURATION
#define CONFIGURATION



void saveBoardConfig(){
    String datasetup;
    JsonDocument doc;

    doc["alpRemPort"] = Config.alpacaPort.remotePort;
    doc["alpPort"] = Config.alpacaPort.alpacaPort;

    serializeJson(doc, datasetup);
    File file = SPIFFS.open("/config.txt", FILE_WRITE);
    file.print(datasetup);
    file.close();
}


void saveConfig(){

    if(Config.save.board.execute){
        saveBoardConfig();
    }
    
    #ifdef DOME 
    if(Config.save.dome.execute){
        saveDomeConfig();
    }
    #endif

    #ifdef SWITCH
    if(Config.save.switches.execute){
        saveSwitchConfig();
    }
    #endif

    #ifdef COVERC
    if(Config.save.coverC.execute){  
        saveCoverCConfig();
    }
    #endif

    #ifdef LIDAR
    if(Config.save.lidar.execute){  
        saveLidarConfig();
    }
    #endif

    Config.save.dome.execute = false;
    Config.save.switches.execute = false;
    Config.save.coverC.execute = false;
    Config.save.lidar.execute = false;

    if(Config.save.coverC.restartNeeded || Config.save.dome.restartNeeded || Config.save.switches.restartNeeded || Config.save.board.restartNeeded){
        Serial.println("restarting...");
        ESP.restart();
    }

}

#endif