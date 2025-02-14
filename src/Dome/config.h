#ifndef DOME_CONFIG
#define DOME_CONFIG


void saveDomeConfig(){

    File file = LittleFS.open("/cfg/domecfg.txt", FILE_WRITE);
    if (!file) {
        Serial.println("Error during open Dome config file");
        Dome.config.Save.execute = false;
        return;
    }
    JsonDocument doc;
 
    JsonObject pinOpen = doc["pinOpen"].to<JsonObject>();
    pinOpen["pin"] = Dome.config.data.inOpen.pin;
    pinOpen["dOn"] = DomeInOpen.getTOn();
    pinOpen["dOff"] = DomeInOpen.getTOff();
    pinOpen["type"] = Dome.config.data.inOpen.type;

    JsonObject pinClose = doc["pinClose"].to<JsonObject>();
    pinClose["pin"] = Dome.config.data.inClose.pin;
    pinClose["dOn"] = DomeInClose.getTOn();
    pinClose["dOff"] = DomeInClose.getTOff();
    pinClose["type"] = Dome.config.data.inClose.type;

    JsonObject autoclose = doc["autoclose"].to<JsonObject>();
    autoclose["enable"] = Dome.config.data.enAutoClose;
    autoclose["minutes"] = Dome.config.data.autoCloseTimeOut;

    doc["pinStart"] = Dome.config.data.outStart_Open;
    doc["pinHalt"] = Dome.config.data.outHalt_Close;
    doc["movTimeOut"] = Dome.config.data.movingTimeOut;

    serializeJson(doc, file);

    
    file.close();
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
    OpenConfig.invert = pinOpen["type"];
    OpenConfig.toff = pinOpen["dOff"];
    OpenConfig.ton = pinOpen["dOn"];

    DomeInOpen.setup(&OpenConfig);

    JsonObject pinClose = doc["pinClose"];

    DigitalInputConfig CloseConfig;

    CloseConfig.pin = pinOpen["pin"];
    CloseConfig.invert = pinOpen["type"];
    CloseConfig.toff = pinOpen["dOff"];
    CloseConfig.ton = pinOpen["dOn"];

    DomeInClose.setup(&CloseConfig);

    JsonObject autoClose = doc["autoclose"];
    Dome.config.data.enAutoClose = autoClose["enable"];
    Dome.config.data.autoCloseTimeOut = autoClose["minutes"];

    DomeOutMoveOpen.setup(doc["pinStart"]);
    DomeOutHaltClose.setup(doc["pinHalt"]);

    Dome.config.data.movingTimeOut = doc["movTimeOut"];

    Dome.config.Load.isValid = true;

}


#endif