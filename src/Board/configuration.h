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
    ip[0] = Global.config.wifi.ip.ip[0];
    ip[1] = Global.config.wifi.ip.ip[1];
    ip[2] = Global.config.wifi.ip.ip[2];
    ip[3] = Global.config.wifi.ip.ip[3];
    JsonArray sub = wifi["sub"].to<JsonArray>();
    sub[0] = Global.config.wifi.ip.sub[0];
    sub[1] = Global.config.wifi.ip.sub[1];
    sub[2] = Global.config.wifi.ip.sub[2];
    sub[3] = Global.config.wifi.ip.sub[3];

    File file = LittleFS.open("/cfg/boarcfg.txt",FILE_WRITE);

    if (!file) {
        Serial.println("Error during open board config file");
        return;
    }

    serializeJson(doc,file);

    file.close();
    Global.config.save.execute = false;
    
}


void initBoardConfig(){

    Serial.println("INIT: Reading Board config...");
    File file = LittleFS.open("/cfg/boarcfg.txt",FILE_READ);

    if (!file) {
        Serial.println("Error during open board config file");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);

    if(error){
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    Global.config.language.locale = doc["locale"].as<String>(); 
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
    
    if(Global.config.wifi.ip.enable){
        JsonArray ip = wifi["ip"];
        Global.config.wifi.ip.ip[0] = ip[0].as<unsigned int>();
        Global.config.wifi.ip.ip[1] = ip[1].as<unsigned int>();
        Global.config.wifi.ip.ip[2] = ip[2].as<unsigned int>();
        Global.config.wifi.ip.ip[3] = ip[3].as<unsigned int>();
        Serial.print("INIT: Board required ip: ");
        Serial.print(Global.config.wifi.ip.ip[0]);
        Serial.print(".");
        Serial.print(Global.config.wifi.ip.ip[1]);
        Serial.print(".");
        Serial.print(Global.config.wifi.ip.ip[2]);
        Serial.print(".");
        Serial.println(Global.config.wifi.ip.ip[3]);
        JsonArray sub = wifi["sub"];
        Global.config.wifi.ip.sub[0] = sub[0].as<unsigned int>();
        Global.config.wifi.ip.sub[1] = sub[1].as<unsigned int>();
        Global.config.wifi.ip.sub[2] = sub[2].as<unsigned int>();
        Global.config.wifi.ip.sub[3] = sub[3].as<unsigned int>();
        Serial.print("INIT: Board subnet ip: ");
        Serial.print(Global.config.wifi.ip.sub[0]);
        Serial.print(".");
        Serial.print(Global.config.wifi.ip.sub[1]);
        Serial.print(".");
        Serial.print(Global.config.wifi.ip.sub[2]);
        Serial.print(".");
        Serial.println(Global.config.wifi.ip.sub[3]);
    }

    for (int i = 0; i < 40; i++)
    {
        Global.gpio[i].module = noModule;
        Global.gpio[i].type = notUsed;

    }
    
}

#endif