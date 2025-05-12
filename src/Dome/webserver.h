#ifndef DOME_SERVER
#define DOME_SERVER


void domeWebServer(){

    server.on("/api/dome-getconfig",               HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->print("{\"dome\":{ \"pinstart\":");
        response->print(Config.dome.pinStart);
        response->print(",\"pinhalt\":");
        response->print(Config.dome.pinHalt);
        response->print(",\"pinopen\":");
        response->print(Config.dome.pinOpen);
        response->print(",\"pinclose\":");
        response->print(Config.dome.pinClose);
        response->print(",\"pinmanual\":");
        response->print(Config.dome.pinManual);
        response->print(",\"tout\":");
        response->print(Config.dome.movingTimeOut);
        response->print(",\"enautoclose\":");
        Config.dome.enAutoClose ? response->print("true") : response->print("false");
        response->print(",\"autoclose\":");
        response->print(Config.dome.autoCloseTimeOut);
        response->print(",\"mountip\":\"");
        response->print(Config.dome.mountIP);
        response->print("\",\"mountport\":");
        response->print(Config.dome.mountPort);
        response->print(",\"seccheck\":");
        Config.dome.secCheck ? response->print("true") : response->print("false");
        response->print(",\"secstrict\":");
        Config.dome.secStrict ? response->print("true") : response->print("false");
        response->print("}}");

        request->send(response);
    });

    AsyncCallbackJsonWebHandler *domecfg = new AsyncCallbackJsonWebHandler("/api/dome-saveconfig", [](AsyncWebServerRequest * request, JsonVariant & json) {
        JsonDocument doc;
        doc = json.as<JsonObject>();
        bool reboot = false;
        if (Config.dome.pinStart != doc["pinstart"]) {reboot=true;}
        Config.dome.pinStart = doc["pinstart"];
        if (Config.dome.pinHalt != doc["pinhalt"]) {reboot=true;}
        Config.dome.pinHalt = doc["pinhalt"];
        if (Config.dome.pinOpen != doc["pinopen"]) {reboot=true;}
        Config.dome.pinOpen = doc["pinopen"];
        if (Config.dome.pinClose != doc["pinclose"]) {reboot=true;}
        Config.dome.pinClose = doc["pinclose"];
        if (Config.dome.pinManual != doc["pinmanual"]) {reboot=true;}
        Config.dome.pinManual = doc["pinmanual"];
        
        Config.dome.movingTimeOut = doc["tout"];
        Config.dome.enAutoClose = doc["enautoclose"];
        Config.dome.autoCloseTimeOut = doc["autoclose"];
        Config.dome.mountIP = doc["mountip"].as<String>();
        Config.dome.mountPort = doc["mountport"];
        Config.dome.secCheck = doc["seccheck"];
        Config.dome.secStrict = doc["secstrict"];
        Config.save.dome.execute = true;
        if (reboot){
            Config.save.dome.restartNeeded = true;
            request->send(200, "application/json", "{\"reboot\": \"1\"}");
        } else {
            request->send(200, "application/json", "{\"accept\": \"ok\"}");
        }
    });

    server.on("/api/dome-cmd",               HTTP_PUT, [](AsyncWebServerRequest *request) {
        if (request->hasParam("cmd")){
            int cmd;
            cmd = request->getParam("cmd")->value().toInt();
            switch (cmd) {
                case 1:
                    if(Dome.ShutterState != ShOpen){
                    Dome.ShutterCommand = CmdOpen;
                    request->send(200, "application/json", "{\"error\":0}");
                    } else { request->send(200, "application/json", "{\"error\":1}"); }
                    break;

                case 2:
                    if(Dome.ShutterState != ShClose){
                        Dome.ShutterCommand = CmdClose;
                        request->send(200, "application/json", "{\"error\":0}");
                    } else { request->send(200, "application/json", "{\"error\":2}"); }
                    break;

                case 3:
                    Dome.ShutterCommand = CmdHalt;
                    Dome.Cycle = 100;
                    request->send(200, "application/json", "{\"error\":0}");
                    break;
                default:
                    request->send(200, "application/json", "{\"error\":3}");
                    break;
            }
        } else {
            request->send(200, "application/json", "{\"error\":3}");
        }
    });


    server.on("/api/dome",               HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        Dome.lastCommunicationMillis  = millis();
        response->printf("{\"dome\":{ \"actualState\":");
        response->print(Dome.ShutterState);
        response->print(",\"lastCommand\":");
        response->print(Dome.LastDomeCommand);
        response->print(",\"safe\":");
        response->print(Dome.Safe);
        response->print("}");
        response->print("}");
        request->send(response);
    });


    server.addHandler(domecfg);

    server.serveStatic("/domeconfig.txt", SPIFFS, "/domeconfig.txt");
}

#endif