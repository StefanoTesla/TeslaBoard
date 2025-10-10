#ifndef SWITCH_CONFIG
#define SWITCH_CONFIG

#define SW_SCHEMA 1


void SWdebug(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Serial.print("[SW] "); 
    Serial.println(buffer);    
}


//since nvs have a string limit I need to split switchs in different parameter sections
void saveSwitchConfig(){
    Switch.config.save.execute=false;   
    SWdebug("Saving data operation started...");
    Preferences preferences;
    preferences.begin("swconfig", false);
    SWdebug("NVS open");
    //clean everything
    preferences.clear();

    //header
    preferences.putBool("enable",true);
    preferences.putInt("schema",SW_SCHEMA);
    SWdebug("Header Saved");

    //single switches
    int i = 0;
    for (JsonObject Switche : tmpSwitchCfg["Switches"].as<JsonArray>()) {
        SWdebug("Saving Switch id: %d",i);
        String jsonStr;
        serializeJson(Switche, jsonStr);
        String key = "sw_"+ String(i);
        preferences.putString(key.c_str(),jsonStr);
        i++;
    }
    preferences.putInt("stored_sw",i);
    preferences.end();

    tmpSwitchCfg.clear();
    SWdebug("Saving operations completed");
}


void initSwitchConfig(){
    SWdebug("Init operation started...");

    JsonDocument doc;
    Preferences preferences;
    
    if (!preferences.begin("swconfig", true)){
        SWdebug("Unable to read configuration");
        preferences.end();
        return;
    }


    Switch.config.isEnable= preferences.getBool("enable",false);
    Switch.config.schemaVersion = preferences.getInt("schema",0);
    Switch.config.configuredSwitch = preferences.getInt("stored_sw",0);
    if(!Switch.config.isEnable){
        SWdebug("Module is not enable, aborting init process...");
        preferences.end();
        return;
    }

    if(Switch.config.schemaVersion < SW_SCHEMA){
        SWdebug("Data required an upgrade operation!");
        //to do when is time
    }

    Switch.config.configuredSwitch = preferences.getInt("stored_sw",0);

    if(Switch.config.configuredSwitch == 0){
        SWdebug("No switch configured");
        preferences.end();
        return;
    } 
    SWdebug("Found %d switches", Switch.config.configuredSwitch);

    int channel;
    for (int i = 0; i < Switch.config.configuredSwitch; i++)
    {
        channel = -1;
        String jsonStr = "";
        String key = "sw_" + String(i);
        jsonStr = preferences.getString(key.c_str(),"{}");

        DeserializationError error = deserializeJson(doc, jsonStr);
        if(error){
            SWdebug("deserializeJson() failed: ");
            SWdebug(error.c_str());
            continue;
        }
        if(doc["type"] == static_cast<int>(SwTypeDInput)){
            SwitchObjects[i] = new DigitalInput;
            DigitalInputConfig DiConfig;
            DiConfig.pin=doc["pin"];
            DiConfig.invert=doc["invert"];
            DiConfig.dOn=doc["dOn"];
            DiConfig.dOff=doc["dOff"];
            SwitchObjects[i]->setup(&DiConfig);
            
        } else if(doc["type"] == static_cast<int>(SwTypeDOutput)){
            SwitchObjects[i] = new DigitalOutput;
            DigitalOutputConfig DOConfig;
            DOConfig.pin=doc["pin"];
            DOConfig.invert=doc["invert"];
            SwitchObjects[i]->setup(&DOConfig);
        //PWM Output
        } else if(doc["type"] == static_cast<int>(SwTypePWM)){
            channel=findLedCChannel();
            if(channel>=0){
                SwitchObjects[i] = new PWMOutput;
                PWMOutputConfig PWMConfig;
                PWMConfig.pin=doc["pin"];
                PWMConfig.ledChannel=channel;
                SwitchObjects[i]->setup(&PWMConfig);
            } else {
                SwitchObjects[i] = nullptr;
            }

        //Servo Output
        } else if(doc["type"] == static_cast<int>(SwTypeServo)){
            channel=findLedCChannel(true);
            if(channel>=0){
                SwitchObjects[i] = new ServoOutput;
                ServoOutputConfig ServoConfig;
                ServoConfig.pin = doc["pin"];
                ServoConfig.ledChannel=channel;
                ServoConfig.maxDeg = doc["maxDeg"];
                ServoConfig.closeDeg= doc["closeDeg"];
                ServoConfig.openDeg = doc["openDeg"];
                ServoConfig.movTime = doc["movTime"];
                SwitchObjects[i]->setup(&ServoConfig);
            } else {
                SwitchObjects[i] = nullptr;
            }
        }

        if (SwitchObjects[i] != nullptr) {
            SwitchObjects[i]->setName(doc["name"].as<const char*>());
            SwitchObjects[i]->setDescription(doc["desc"].as<const char*>());
        }

        doc.clear();
    }
    
    Switch.config.load.isValid = true;
}

#endif