#ifndef DOME_CONFIG
#define DOME_CONFIG


void saveDomeConfig(){

    File file = LittleFS.open("/cfg/domecfg.txt", FILE_WRITE);
    if (!file) {
        Serial.println("Error during open Dome config file");
        Dome.config.Save.execute = false;
        return;
    }


    serializeJson(DomeConfigTmp, file);

    
    file.close();

    DomeConfigTmp.clear();
    Dome.config.Save.execute = false;

}


void initDomeConfig(){
    Serial.println("INIT: Reading Dome config...");
    JsonDocument doc;
    File file = LittleFS.open("/cfg/domecfg.txt", FILE_READ);

    if (!file) {
        Serial.println("[ERR] Init: Reading Dome config error");
        saveDomeConfig();
        return;
    }

    DeserializationError error = deserializeJson(doc, file);
    if(error){
        Serial.print(F("[ERR] Init: Reading Dome config deserializeJson() failed: "));
        Serial.println(error.c_str());
        file.close();
        return;
    }
    file.close();

    serializeJson(doc,Serial);
    JsonObject pinOpen = doc["pinOpen"];
    DigitalInputConfig OpenConfig;
    OpenConfig.pin = pinOpen["pin"];
    OpenConfig.type = pinOpen["type"];

    DomeInOpen.setup(&OpenConfig);
    DomeInOpen.dOn = pinOpen["dOn"];
    DomeInOpen.dOff = pinOpen["dOff"];

    JsonObject pinClose = doc["pinClose"];
    DigitalInputConfig CloseConfig;
    CloseConfig.pin = pinClose["pin"];
    CloseConfig.type = pinClose["type"];
    DomeInClose.setup(&CloseConfig);
    DomeInClose.dOn = pinClose["dOn"];
    DomeInClose.dOff = pinClose["dOff"];

    JsonObject autoClose = doc["autoclose"];
    Dome.config.data.enAutoClose = autoClose["enable"];
    Dome.config.data.autoCloseTimeOut = autoClose["minutes"];
    DigitalOutputConfig StartConfig;
    StartConfig.pin = doc["pinStart"];
    DomeOutMoveOpen.setup(&StartConfig);

    DigitalOutputConfig HaltConfig;
    HaltConfig.pin = doc["pinStart"];
    DomeOutHaltClose.setup(&HaltConfig);

    Dome.config.data.movingTimeOut = doc["movTimeOut"];

    Dome.config.Load.isValid = true;

}


#endif