#ifndef SWITCH_CONFIG
#define SWITCH_CONFIG


void saveSwitchConfig(){
    File file = LittleFS.open("/cfg/switchcfg.txt", FILE_WRITE);
    Serial.println("Switch save in progress..");
    serializeJson(tmpSwitchCfg, file);
    tmpSwitchCfg.clear();
    file.close();    
}


void initSwitchConfig(){
    Serial.println("INIT: switch config reading..");
    if (!LittleFS.exists("/cfg/switchcfg.txt")) {
        Serial.println("[ERR] Switch: unable to find switchconfig file, I'm creating a new one..");
        File file = LittleFS.open("/cfg/switchconfig.txt", FILE_WRITE);
        file.close();
        Switch.config.load.isValid = false;
        return;
    }

    File file = LittleFS.open("/cfg/switchcfg.txt", FILE_READ);
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, file);

    if(error){
        file.close();
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        Switch.config.load.isValid = false;
        return;
    }    
    file.close();
    int tmpCh = -1;
    int count = 0;
    for (JsonObject Switche : doc["Switches"].as<JsonArray>()) {
        if(count >= _MAX_SWITCH_ID_){
            Serial.println("[SWI] Too many switches configured");
            exit;
        }
        tmpCh = -1;
        //Digital Input
        if(Switche["type"] == static_cast<int>(SwTypeDInput)){
            SwitchObjects[count] = new DigitalInput;
            DigitalInputConfig DiConfig;
            DiConfig.pin=Switche["pin"];
            DiConfig.invert=Switche["invert"];
            DiConfig.dOn=Switche["dOn"];
            DiConfig.dOff=Switche["dOff"];
            SwitchObjects[count]->setup(&DiConfig);
            
        } else if(Switche["type"] == static_cast<int>(SwTypeDOutput)){
            SwitchObjects[count] = new DigitalOutput;
            DigitalOutputConfig DOConfig;
            DOConfig.pin=Switche["pin"];
            DOConfig.invert=Switche["invert"];
            SwitchObjects[count]->setup(&DOConfig);
        //PWM Output
        } else if(Switche["type"] == static_cast<int>(SwTypePWM)){
            tmpCh = assignLedChannel(pwm);
            if(tmpCh >= 0 && tmpCh < 16){
                SwitchObjects[count] = new PWMOutput;
                PWMOutputConfig PWMConfig;
                PWMConfig.pin=Switche["pin"];
                PWMConfig.channel=tmpCh;
                SwitchObjects[count]->setup(&PWMConfig);
            }
        //Servo Output
        } else if(Switche["type"] == static_cast<int>(SwTypeServo)){
            tmpCh = assignLedChannel(servo);
            if(tmpCh >= 0 && tmpCh < 16){
                SwitchObjects[count] = new ServoOutput;
                ServoOutputConfig ServoConfig;
                ServoConfig.pin = Switche["pin"];
                ServoConfig.channel = tmpCh;
                ServoConfig.maxDeg = Switche["maxDeg"];
                ServoConfig.closeDeg= Switche["closeDeg"];
                ServoConfig.openDeg = Switche["openDeg"];
                ServoConfig.movTime = Switche["movTime"];

                SwitchObjects[count]->setup(&ServoConfig);
            }
        }

        if (SwitchObjects[count] != nullptr) {
            SwitchObjects[count]->setName(Switche["name"].as<const char*>());
            SwitchObjects[count]->setDescription(Switche["desc"].as<const char*>());
        }
        count +=1;

    }

    Switch.config.configuredSwitch = count;

}

#endif