#ifndef CC_CONFIG
#define CC_CONFIG

void saveCoverCConfig(){

    File file = LittleFS.open("/cfg/cccfg.txt", FILE_WRITE);
    CoverC.config.save.execute=false;
    
    if (!file) {
        Serial.println("Error during open CoverCalibration config file");
        return;
    }

    serializeJson(CoverCConfigTmp, file);
    serializeJson(CoverCConfigTmp,Serial);
    CoverCConfigTmp.clear();
    
    file.close();

}

void initCoverCConfig(){
    Serial.println("INIT: Reading cover calibrator config...");
    JsonDocument doc;
    File file = LittleFS.open("/cfg/cccfg.txt", FILE_READ);
    if (!file) {
        Serial.println("[ERR] INIT: Reading Cover Calibrator config error");
        return;
    }
    DeserializationError error = deserializeJson(doc, file);
    serializeJson(doc,Serial);
    if(error){
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        CoverC.config.load.isValid = false;
        return;
    }
    file.close();

    int tmpCh = -1;
    JsonObject calibrator = doc["calibrator"];
    CoverC.config.calibrator.present = calibrator["present"];

    JsonObject cover = doc["cover"];
    CoverC.config.cover.present = cover["present"];
    
    
    if(CoverC.config.calibrator.present){ 
        tmpCh = assignLedChannel(pwm);
        if(tmpCh >= 0 && tmpCh < 16){
            PWMOutputConfig CalibConfig;
            CalibConfig.pin = calibrator["pin"];
            CalibConfig.channel = tmpCh;
            Calibrator.setup(&CalibConfig);
        } 
    }

    tmpCh = -1;

    if(CoverC.config.cover.present){
    tmpCh = assignLedChannel(servo);
        if(tmpCh >= 0 && tmpCh < 16){
            ServoOutputConfig CoverConfig;
            CoverConfig.pin = cover["pin"];
            CoverConfig.channel = tmpCh;
            Cover.setup(&CoverConfig);
            Cover.movingTime = cover["movingTime"];
            Cover.openDeg = cover["openDeg"];
            Cover.closeDeg = cover["closeDeg"];
            Cover.maxDeg = cover["maxDeg"];
        }
    }

    CoverC.config.load.isValid = true;

}


#endif