#ifndef SWITCH_CONFIG
#define SWITCH_CONFIG


void saveSwitchConfig(){
    File file = LittleFS.open("/cfg/switchcfg.txt", FILE_WRITE);
    Serial.println("Switch save in progress..");
    logMessage(Switches,lInfo,"Saving config file");
    JsonDocument doc;
    JsonArray array = doc["Switches"].to<JsonArray>();

    for (size_t i = 0; i < _MAX_SWITCH_ID_ ; i++)
    {
        if(Switch.config.tmp[i].property.type == SwTypeNull  ){
            continue;
        }
        JsonObject jsonSwitch = array.add<JsonObject>();
        jsonSwitch["name"] = Switch.config.tmp[i].property.Name;
        jsonSwitch["desc"] = Switch.config.tmp[i].property.Description;
        jsonSwitch["type"] = Switch.config.tmp[i].property.type;

        switch(Switch.config.tmp[i].property.type){
            case SwTypeServo:

                jsonSwitch["min"] = Switch.config.tmp[i].property.minValue;
                jsonSwitch["max"] = Switch.config.tmp[i].property.maxValue;

            case SwTypePWM:
            case SwTypeAInput:
            case SwTypeAOutput:
            case SwTypeDInput:
            case SwTypeDOutput:
                jsonSwitch["pin"] = Switch.config.tmp[i].property.pin;
                break;
            default:
                break;
        }
    }

    serializeJson(doc, file);
    file.close();
    logMessage(Switches,lInfo,"Config saved");

    
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
        //Digital Input
        if(Switche["type"] == 1){
            SwitchObjects[count] = new DigitalInput;
            DigitalInputConfig DiConfig;
            DiConfig.pin=Switche["pin"];
            SwitchObjects[count]->setup(&DiConfig);
        //Analog Input - not really know if will develop
        } else if(Switche["type"] == 2){
            Serial.println("gne");
        //Digital Output
        } else if(Switche["type"] == 3){
            SwitchObjects[count] = new DigitalOutput;
            DigitalOutputConfig DOConfig;
            DOConfig.pin=Switche["pin"];
            SwitchObjects[count]->setup(&DOConfig);
        //PWM Output
        } else if(Switche["type"] == 4){
            tmpCh = -1;
            tmpCh = assignLedChannel(pwm);
            if(tmpCh >= 0 && tmpCh < 16){
                SwitchObjects[count] = new PWMOutput;
                PWMOutputConfig PWMConfig;
                PWMConfig.pin=Switche["pin"];
                PWMConfig.channel=tmpCh;
                SwitchObjects[count]->setup(&PWMConfig);
            }
        //Servo Output
        } else if(Switche["type"] == 5){
            tmpCh = -1;
            tmpCh = assignLedChannel(servo);
            if(tmpCh >= 0 && tmpCh < 16){
                SwitchObjects[count] = new ServoOutput;
                ServoOutputConfig ServoConfig;
                ServoConfig.pin=Switche["pin"];
                ServoConfig.channel=tmpCh;
                SwitchObjects[count]->setup(&ServoConfig);
            }
        }

        if (SwitchObjects[count] != nullptr) {
            SwitchObjects[count]->setName(Switche["name"].as<const char*>());
            SwitchObjects[count]->setDescription(Switche["desc"].as<const char*>());
        }

        tmpCh = -1;
        count +=1;

    }

    Switch.config.configuredSwitch = count;

}

#endif