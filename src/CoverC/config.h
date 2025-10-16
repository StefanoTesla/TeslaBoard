#ifndef CC_CONFIG
#define CC_CONFIG

#define COVERC_SCHEMA 1

void CoverCdebug(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Serial.print("[CC] "); 
    Serial.println(buffer);    
}


void saveCoverCConfig(){
    CoverC.config.save.execute=false;

    Preferences preferences;
    preferences.begin("ccconfig", false);

    String jsonStr;
    serializeJson(CoverCConfigTmp, jsonStr);
    preferences.putString("settings", jsonStr);
    preferences.end();
    CoverCConfigTmp.clear();

}

void initCoverCConfig(){
    Serial.println("INIT: Reading cover calibrator config...");
    JsonDocument doc;

    Preferences preferences;

    preferences.begin("ccconfig", true);
    String jsonStr = preferences.getString("settings");

    DeserializationError error = deserializeJson(doc, jsonStr);
    if(error){
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        CoverC.config.load.isValid = false;
        return;
    }
    preferences.end();

    CoverC.config.isEnable = preferences.getBool("enable", false);

    if(!CoverC.config.isEnable){
        //DomeDebug("module not enable, aborting init process...");
        preferences.end();
        return;
    }

    CoverC.config.schemaVersion = preferences.getInt("schema",1);


    if(CoverC.config.schemaVersion < COVERC_SCHEMA){
        //DomeDebug("Data required an upgrade operation!");
        //to do when is time
    }

    CoverC.config.order = preferences.getInt("order",1);


    JsonObject calibrator = doc["calibrator"];
    CoverC.config.calibrator.present = calibrator["present"];

    JsonObject cover = doc["cover"];
    CoverC.config.cover.present = cover["present"];
    
    int channel = -1;

    if(CoverC.config.calibrator.present){ 

        channel = findLedCChannel();

        if(channel >= 0){
            PWMOutputConfig CalibConfig;
            CalibConfig.pin = calibrator["pwm"]["pin"];
            CalibConfig.ledChannel = channel;
            CalibConfig.fastPWM = true;
            Calibrator.setup(&CalibConfig);
        } else {
            Serial.println("Unable to find a ledC channel for calibrator");
            CoverC.config.calibrator.present = false;
        }

    }



    if(CoverC.config.cover.present){

        channel = findLedCChannel(true);

        if(channel >=0){
            JsonObject se = cover["servo"];

            ServoOutputConfig CoverConfig;
            CoverConfig.ledChannel = channel;
            CoverConfig.pin = se["pin"];
            CoverConfig.maxDeg = se["maxDeg"];
            CoverConfig.closeDeg = se["closeDeg"]; 
            CoverConfig.openDeg = se["openDeg"];
            CoverConfig.movTime = se["movTime"];
            Cover.setup(&CoverConfig);
        } else {
            Serial.println("Unable to find a ledC channel for cover");
            CoverC.config.cover.present = false;
        }


    }

    CoverC.config.load.isValid = true;

}

AsyncMiddlewareFunction isCCEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(CoverC.config.isEnable){
        next();
    }
    request->send(500, "text/plain", "Module not enabled");
});



AsyncMiddlewareFunction isCoverCEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(CoverC.config.isEnable){
        next();
    }
    request->send(500, "text/plain", "Module not enabled");
});
#endif