#ifndef DOME_CONFIG
#define DOME_CONFIG


void saveDomeConfig(){

    File file = LittleFS.open("/cfg/domecfg.txt", FILE_WRITE);
    if (!file) {
        Serial.println("Error during open Dome config file");
        return;
    }

    DomeConfigTmp.remove("reboot");

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

    JsonObject pinOpen = doc["pinOpen"];

    DigitalInputConfig OpenConfig;
    OpenConfig.pin = pinOpen["pin"];
    OpenConfig.invert = pinOpen["invert"];
    DomeInOpen.setup(&OpenConfig);
    DomeInOpen.dOn = pinOpen["dOn"];
    DomeInOpen.dOff = pinOpen["dOff"];
    assingPin(OpenConfig.pin,input,domeModule);

    JsonObject pinClose = doc["pinClose"];
    DigitalInputConfig CloseConfig;
    CloseConfig.pin = pinClose["pin"];
    CloseConfig.invert = pinClose["invert"];
    DomeInClose.setup(&CloseConfig);
    DomeInClose.dOn = pinClose["dOn"];
    DomeInClose.dOff = pinClose["dOff"];
    assingPin(CloseConfig.pin,input,domeModule);

    JsonObject pinStart = doc["pinStart"];
    DigitalOutputConfig StartConfig;
    StartConfig.pin = pinStart["pin"];
    StartConfig.invert = pinStart["invert"];
    DomeOutMoveOpen.setup(&StartConfig);
    assingPin(StartConfig.pin,input,domeModule);

    JsonObject pinHalt = doc["pinHalt"];
    DigitalOutputConfig HaltConfig;
    HaltConfig.pin = pinHalt["pin"];
    HaltConfig.invert = pinHalt["invert"];
    DomeOutHaltClose.setup(&HaltConfig);
    assingPin(HaltConfig.pin,input,domeModule);

    /* Dome strict configuration */
    JsonObject autoClose = doc["autoclose"];
    Dome.config.data.enAutoClose = autoClose["enable"];
    Dome.config.data.autoCloseTimeOut = autoClose["minutes"];

    Dome.config.data.driverType = doc["driverType"];
    Dome.config.data.movingTimeOut = doc["movTimeOut"];
    Dome.config.Load.isValid = true;

    //reset the output
    DomeOutMoveOpen.write(0);
    DomeOutHaltClose.write(0);


}


#endif