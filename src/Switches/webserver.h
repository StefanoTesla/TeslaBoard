#ifndef SWITCH_SERVER
#define SWITCH_SERVER

void switchWebServer(){

    server.on("/api/switch-cmd",             HTTP_PUT, [](AsyncWebServerRequest *request) {
        int id = -1;
        int value = -1;
        if (request->hasParam("id")){
             id = request->getParam("id")->value().toInt();
        } else { 
            request->send(200, "application/json", "{\"error\" : \"Missing ID\"}");
            return;
        }
        if (request->hasParam("value")){ 
            value = request->getParam("value")->value().toInt();
        } else {
            request->send(200, "application/json", "{\"error\" : \"Missing Value\"}");
            return;
        }
        if (id < 0 || id >= Config.switches.configuredSwitch){ 
            request->send(200, "application/json", "{\"error\" : \"ID Out of Range\"}"); return;
        }
        if (Switch[id].property.type != 1 &&  Switch[id].property.type != 3){ 
            request->send(200, "application/json", "{\"error\" : \"Switch cannot be setted\"}"); 
            return;
        }
        if (!validateSwitchValue(id,value)){
            request->send(200, "application/json", "{\"error\" : \"Value outside limits\"}");
            return;
        }

        Switch[id].cmdValue = value;
        request->send(200, "text/html", "{\"done\" : 1}");
    
    });

    AsyncCallbackJsonWebHandler *switchConfig = new AsyncCallbackJsonWebHandler("/api/switch-saveconfig", [](AsyncWebServerRequest * request, JsonVariant & json) {
        JsonDocument doc;
        int i=0;
        int x=0;
        int type = 0;
        int pin = 0;
        bool err = false;
        bool reboot = false;
        for (JsonObject elem : json.as<JsonArray>()) {
            type = elem["type"].as<unsigned int>();
            pin = elem["pin"].as<unsigned int>();
            if((type > 0 && type <= 4) && pin < _MAXPIN && validatePin(pin,type)){
                if((pin != Switch[i].pin && pin != 0) || type != Switch[i].property.type) { reboot = true; } //avoid reboot if delete switch
                Switch[i].pin = pin;
                Switch[i].property.type = type;
                Switch[i].Name = elem["name"].as<String>();
                Switch[i].Description = elem["desc"].as<String>();
            } else {
                err = true;
                continue;
            }
            i++;
        }
        if(err){
            request->send(200, "application/json", "{\"error\": \"some type or input are wrong\"}");
            return;
        }
        for(x=i;x<_MAX_SWITCH_ID_;x++){
            Switch[x].Name = "";
            Switch[x].Description = "";
            Switch[x].pin = 0;
            Switch[x].property.type = 0;
            Switch[x].property.minValue = 0;
            Switch[x].property.maxValue = 1;
        }

        Config.switches.configuredSwitch = i;

        
        Config.save.switches.execute = true;
        if (reboot){
            Config.save.switches.restartNeeded = true;
            request->send(200, "application/json", "{\"reboot\": \"1\"}");
        } else {
            request->send(200, "application/json", "{\"accept\": \"1\"}");
        }

    });
    server.addHandler(switchConfig);
    
    server.on("/api/switch-getconfig",                HTTP_GET, [](AsyncWebServerRequest *request) {
        int i;
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->print("{");
        response->print("\"switches\":[");
        for(i=0;i<Config.switches.configuredSwitch;i++){
            response->print("{\"name\":\"");
            response->print(Switch[i].Name);
            response->print("\",\"desc\":\"");
            response->print(Switch[i].Description);
            response->print("\",\"type\":");
            response->print(Switch[i].property.type);
            response->print(",\"pin\":");
            response->print(Switch[i].pin);
            response->print("}");
            if (i != Config.switches.configuredSwitch -1){
                response->print(",");
            }
        }    
        response->print("]");

        response->print("}");
        request->send(response);
    });

    server.on("/api/switch",                HTTP_GET, [](AsyncWebServerRequest *request) {
    int i;
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->printf("{");
    response->print("\"switches\": [");
    for (i=0;i<Config.switches.configuredSwitch;i++){
        response->print("{\"id\":");
        response->print(i);
        response->print(",\"name\":\"");
        response->print(Switch[i].Name);
        response->print("\",\"description\":\"");
        response->print(Switch[i].Description);
        response->print("\",\"actualValue\":");
        response->print(Switch[i].readValue);
        response->print(",\"type\":");
        response->print(Switch[i].property.type);
        response->print(",\"min\":");
        response->print(Switch[i].property.minValue);
        response->print(",\"max\":");
        response->print(Switch[i].property.maxValue);
        response->print(",\"step\":");
        response->print(Switch[i].property.Step);
        response->print("}");
        if(i != Config.switches.configuredSwitch -1 ){
            response->printf(",");
        }
    }
    response->printf("]}");
    request->send(response);
    });

    server.serveStatic("/switchconfig.txt", SPIFFS, "/switchconfig.txt");
}
#endif