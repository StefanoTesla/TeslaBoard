#ifndef BOARD_CONFIG
#define BOARD_CONFIG

void saveBoardConfig(){
    Serial.println("BOARD: save config request");
    JsonDocument doc;

    doc["locale"] = Global.config.language.locale;
    
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["reconTime"] = Global.config.wifi.reconnection.intervall;
    wifi["enStaticIP"] = Global.config.wifi.ip.enable;

    JsonArray ip = wifi["ip"].to<JsonArray>();
    for (size_t i = 0; i < 4; i++)
    {
        ip[i] = Global.config.wifi.ip.ip[i];
    }

    JsonArray sub = wifi["sub"].to<JsonArray>();
    for (size_t i = 0; i < 4; i++)
    {
        sub[i] = Global.config.wifi.ip.sub[i];
    }

    Preferences preferences;
    String jsonStr;
    serializeJson(doc,jsonStr);
    preferences.begin("boardconfig", false);
    preferences.putString("settings", jsonStr);
    preferences.end();
    Global.config.save.execute = false;
    
}


void initBoardConfig(){

    Serial.println("INIT: Reading Board config...");
    JsonDocument doc;
    Preferences preferences;

    preferences.begin("boardconfig", true);
    String jsonStr = preferences.getString("settings");
    preferences.end();
    DeserializationError error = deserializeJson(doc, jsonStr);
    if(error){
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());

        return;
    }

    Global.config.language.locale = doc["locale"].as<String>(); 
    Global.config.language.locale = "it";
    Serial.print("INIT: Board Locale: ");
    Serial.println(Global.config.language.locale);
    JsonObject wifi = doc["wifi"];

    Global.config.wifi.reconnection.intervall = wifi["reconTime"].as<unsigned int>();
    Serial.print("INIT: Board wifi reconnection time: ");
    Serial.println(Global.config.wifi.reconnection.intervall);

    Global.config.wifi.ip.enable = wifi["enStaticIP"].as<bool>();

    Serial.print("INIT: Board wifi static ip address enable: ");
    if(Global.config.wifi.ip.enable){
        Serial.println("true");
    } else {
        Serial.println("false");
    }
    

    for (int i = 0; i < 40; i++)
    {
        Global.gpio[i].module = noModule;
        Global.gpio[i].type = notUsed;

    }
    
}

#endif