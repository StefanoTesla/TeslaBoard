#ifndef CC_CONFIG
#define CC_CONFIG

void saveCoverCConfig(){

    File file = LittleFS.open("/cfg/cccfg.txt", FILE_WRITE);

    if (!file) {
        Serial.println("Error during open CoverCalibration config file");
        return;
    }
    JsonDocument doc;
    
    JsonObject cover = doc["Cover"].to<JsonObject>();
    cover["present"] = CoverC.config.tmpCfg.cover.present;
    cover["pin"] = CoverC.config.tmpCfg.cover.outServoPin;
    cover["movingTime"] = CoverC.config.tmpCfg.cover.movingTime;
    cover["openDeg"] = CoverC.config.tmpCfg.cover.openDeg;
    cover["closeDeg"] = CoverC.config.tmpCfg.cover.closeDeg;
    cover["maxDeg"] = CoverC.config.tmpCfg.cover.maxDeg;

    JsonObject calibrator = doc["Calibrator"].to<JsonObject>();
    calibrator["present"] = CoverC.config.tmpCfg.calibrator.present;
    calibrator["pin"] = CoverC.config.tmpCfg.calibrator.outPWM;

    serializeJson(doc, file);
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
    
    if(error){
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        CoverC.config.load.isValid = false;
        return;
    }
    file.close();
    JsonObject calibrator = doc["Calibrator"];
    CoverC.config.calibrator.present = calibrator["present"];

    JsonObject cover = doc["Cover"];
    CoverC.config.cover.present = cover["present"];
    CoverC.config.cover.outServoPin = cover["pin"];
    CoverC.config.cover.movingTime = cover["movingTime"];
    CoverC.config.cover.openDeg = cover["openDeg"];
    CoverC.config.cover.closeDeg = cover["closeDeg"];
    CoverC.config.cover.maxDeg = cover["maxDeg"];
    
    unsigned int tmpCh = 666;
    if(CoverC.config.calibrator.present){ 
        tmpCh = assignLedChannel(pwm);
        if(tmpCh < 16){
            Calibrator.setup(calibrator["pin"],tmpCh);
        } 
    }

    tmpCh = 666;

    if(CoverC.config.cover.present){
    tmpCh = assignLedChannel(servo);
        if(tmpCh < 16){
            CoverC.config.cover.pwmChannel = tmpCh;
            
            ledcAttachPin(CoverC.config.cover.outServoPin, tmpCh);
        }
    }

    CoverC.config.load.isValid = true;

}


#endif