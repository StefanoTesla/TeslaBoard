#ifndef DOME_CONFIG
#define DOME_CONFIG

#define DOME_SCHEMA 1

void DomeDebug(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Serial.print("[DOME] "); 
    Serial.println(buffer);    
}

void saveDomeConfig(){

    Dome.config.Save.execute = false;
    Preferences preferences;
    DomeDebug("Starting saving operation...");
    preferences.begin("domeconfig", false);

    //header
    preferences.putBool("enable", true);
    preferences.putInt("schema", DOME_SCHEMA);


    //all the setting (we are near 300bytes far away from the 1900bytes limit)
    String jsonStr;
    serializeJson(DomeConfigTmp, jsonStr);
    preferences.putString("settings", jsonStr);
    preferences.end();

    DomeConfigTmp.clear();
    
}

void initDomeConfig(){
    DomeDebug("Init operation started...");
    JsonDocument doc;
    Preferences preferences;

    if(!preferences.begin("domeconfig", true)){
        DomeDebug("Unable to read configuration");
        preferences.end();
        return;
    };

    Dome.config.isEnable = preferences.getBool("enable", false);

    if(!Dome.config.isEnable){
        DomeDebug("module not enable, aborting init process...");
        preferences.end();
        return;
    }

    Dome.config.schemaVersion = preferences.getInt("schema",1);

    if(Dome.config.schemaVersion < DOME_SCHEMA){
        DomeDebug("Data required an upgrade operation!");
        //to do when is time
    }

    String jsonStr = preferences.getString("settings");
    preferences.end();

    DeserializationError error = deserializeJson(doc, jsonStr);
    if(error){
        DomeDebug("[ERR] Init: Reading Dome config deserializeJson() failed: ");
        DomeDebug(error.c_str());
        preferences.end();
        return;
    }


    JsonObject pinOpen = doc["pinOpen"];

    DigitalInputConfig OpenConfig;
    OpenConfig.pin = pinOpen["pin"];
    OpenConfig.dOn = pinOpen["dOn"];
    OpenConfig.dOff = pinOpen["dOff"];
    OpenConfig.invert = pinOpen["invert"];
    DomeInOpen.setup(&OpenConfig);

    JsonObject pinClose = doc["pinClose"];
    DigitalInputConfig CloseConfig;
    CloseConfig.pin = pinClose["pin"];
    CloseConfig.invert = pinClose["invert"];
    CloseConfig.dOn = pinClose["dOn"];
    CloseConfig.dOff = pinClose["dOff"];
    DomeInClose.setup(&CloseConfig);


    JsonObject pinStart = doc["pinStart"];
    DigitalOutputConfig StartConfig;
    StartConfig.pin = pinStart["pin"];
    StartConfig.invert = pinStart["invert"];
    DomeOutMoveOpen.setup(&StartConfig);

    JsonObject pinHalt = doc["pinHalt"];
    DigitalOutputConfig HaltConfig;
    HaltConfig.pin = pinHalt["pin"];
    HaltConfig.invert = pinHalt["invert"];
    DomeOutHaltClose.setup(&HaltConfig);

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


AsyncMiddlewareFunction isDomeEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(Dome.config.isEnable){
        next();
    }
    request->send(500, "text/plain", "Module not enabled");
});

#endif