#ifndef CC_CONFIG
#define CC_CONFIG

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

    JsonObject calibrator = doc["calibrator"];
    CoverC.config.calibrator.present = calibrator["present"];

    JsonObject cover = doc["cover"];
    CoverC.config.cover.present = cover["present"];
    
    if(CoverC.config.calibrator.present){ 
        PWMOutputConfig CalibConfig;
        CalibConfig.pin = calibrator["pwm"]["pin"];
        CalibConfig.fastPWM = true;
        Calibrator.setup(&CalibConfig);
    }



    if(CoverC.config.cover.present){

    JsonObject se = cover["servo"];

    ServoOutputConfig CoverConfig;
    CoverConfig.pin = se["pin"];
    CoverConfig.maxDeg = se["maxDeg"];
    CoverConfig.closeDeg = se["closeDeg"]; 
    CoverConfig.openDeg = se["openDeg"];
    CoverConfig.movTime = se["movTime"];
    Cover.setup(&CoverConfig);

    }

    CoverC.config.load.isValid = true;

}

AsyncMiddlewareFunction isCCEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(CoverC.config.isEnable){
        next();
    }
    request->send(500, "text/plain", "Module not enabled");
});
#endif