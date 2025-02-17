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

    //reset temporary configuration
    for (int i = 0; i < _MAX_SWITCH_ID_; i++)
    {
        memset(Switch.config.tmp[i].property.Name, 0, sizeof(Switch.config.tmp[i].property.Name));
        memset(Switch.config.tmp[i].property.Description, 0, sizeof(Switch.config.tmp[i].property.Description));
        Switch.config.tmp[i].property.type = SwTypeNull;
        Switch.config.tmp[i].property.minValue =0;
        Switch.config.tmp[i].property.maxValue =0;

    }
    
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
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        Switch.config.load.isValid = false;
        return;
    }    
    file.close();
    int count = 0;
    for (JsonObject Switche : doc["Switches"].as<JsonArray>()) {
        if(Switche["type"] == 1){
            SwitchObjects[count] = new DigitalInput;
            DigitalInputConfig DiConfig;
            DiConfig.pin=Switche["pin"];
            SwitchObjects[count]->setup(&DiConfig);
            Switch.data[count].property.minValue = 0;
            Switch.data[count].property.maxValue = 1;
            SwitchObjects[count]->setName(Switche["name"].as<const char*>());
            SwitchObjects[count]->setDescriprion(Switche["desc"].as<const char*>());
        } else if(Switche["type"] == 2){
            Serial.println("gne");
        } else if(Switche["type"] == 3){
            SwitchObjects[count] = new DigitalOutput;
            DigitalOutputConfig DOConfig;
            DOConfig.pin=Switche["pin"];
            SwitchObjects[count]->setup(&DOConfig);
            Switch.data[count].property.minValue = 0;
            Switch.data[count].property.maxValue = 1;
            SwitchObjects[count]->setName(Switche["name"].as<const char*>());
            SwitchObjects[count]->setDescriprion(Switche["desc"].as<const char*>());
        } else if(Switche["type"] == 4){
            Serial.println("gne");
        } else if(Switche["type"] == 5){
        }
/*
        switch (Switch.data[count].property.type)
        {
        case SwTypeNull:
            break;
        case SwTypeDInput:
            SwitchObjects[count] = new DigitalInput;
            DigitalInputConfig DiConfig;
            DiConfig.pin=Switche["pin"];
            SwitchObjects[count]->setup(&DiConfig);
            Switch.data[count].property.minValue = 0;
            Switch.data[count].property.maxValue = 1;
            break;

        case SwTypeDOutput:
            SwitchObjects[count] = new DigitalOutput;
            SwitchObjects[count]->setPin(Switch.data[count].property.pin)->setup();
            pinMode(Switch.data[count].property.pin,OUTPUT);
            Switch.data[count].property.minValue = 0;
            Switch.data[count].property.maxValue = 1;
            break;
        case SwTypePWM:
            SwitchObjects[count] = new PWMOutput;
            SwitchObjects[count]->setup(Switch.data[count].property.pin);
            Switch.data[count].property.pwmch = assignLedChannel(pwm);
            if(Switch.data[count].property.pwmch < 16){
                ledcAttachPin(Switch.data[count].property.pin, Switch.data[count].property.pwmch);
            } else {
                Serial.println("[ERR] Init: Unable to get a free timer");
            }
            Switch.data[count].property.minValue = 0;
            Switch.data[count].property.maxValue = 4096;
            break;
        case SwTypeServo:
            SwitchObjects[count] = new ServoOutput;
            SwitchObjects[count]->setup();
            //SwitchObjects[count]->goToSlowly()
            Switch.data[count].property.pwmch = assignLedChannel(servo);
            if(Switch.data[count].property.pwmch < 16){
                ledcAttachPin(Switch.data[count].property.pin, Switch.data[count].property.pwmch);
            }
            Switch.data[count].property.minValue = Switche["min"].is<int>() ? Switche["min"] : 0;
            Switch.data[count].property.maxValue = Switche["max"].is<int>() ? Switche["max"] : 180;
            break;
        
        default:
            Serial.println("wrong hole!");
            break;
        }
*/
        count +=1;

    }

    Switch.config.configuredSwitch = count;

}

#endif