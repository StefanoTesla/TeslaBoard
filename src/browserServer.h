#ifndef BROWSER_WEB
#define BROWSER_WEB

void notFound(AsyncWebServerRequest *request) {
  request->send(400, "text/plain", "Not found");
  Serial.println("400");
  Serial.println(request->url());
}


void browserServer(){

    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    server.on("/setup", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(SPIFFS, "/setup.html", "text/html");
    });

    server.on("/status",                HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->printf("{");
        response->printf("\"dome\":");
        #ifdef DOME
            response->printf("1");
        #else
            response->printf("0");
        #endif
            response->printf(",\"switch\":");
        #ifdef SWITCH
            response->printf("1");
        #else
            response->printf("0");
        #endif
            response->printf(",\"cover\":");
        #ifdef COVERC
            response->printf("1");
        #else
            response->printf("0");
        #endif
        response->printf(",\"lidar\":");
        #ifdef LIDAR
            response->printf("1");
        #else
            response->printf("0");
        #endif
        response->printf(",\"freeHeap\":");
        response->print(ESP.getFreeHeap());
        response->printf(",\"minFreeHeap\":");
        response->print(ESP.getMinFreeHeap());
        response->printf(",\"totalHeap\":");
        response->print(ESP.getHeapSize());
        response->printf(",\"upTime\":");
        response->print(Global.esp32.upTime.minutes);
        response->printf(",\"upWiFiTime\":");
        response->print(Global.wifi.upTime.minutes);
        response->print("}");
        request->send(response);
    });

    server.on("/get-config",                HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->print("{");
        response->printf("\"dome\":");
        #ifdef DOME
            response->printf("1");
        #else
            response->printf("0");
        #endif
            response->printf(",\"switch\":");
        #ifdef SWITCH
            response->printf("1");
        #else
            response->printf("0");
        #endif
            response->printf(",\"cover\":");
        #ifdef COVERC
            response->printf("1");
        #else
            response->printf("0");
        #endif
            response->printf(",\"lidar\":");
        #ifdef LIDAR
            response->printf("1");
        #else
            response->printf("0");
        #endif
        response->printf(",\"board\":{\"alpRemPort\":");
        response->print(Config.alpacaPort.remotePort);
        response->printf(",\"alpPort\":");
        response->print(Config.alpacaPort.alpacaPort);
        response->print("}}");
        request->send(response);
    });

    AsyncCallbackJsonWebHandler *boardcfg = new AsyncCallbackJsonWebHandler("/save-config", [](AsyncWebServerRequest * request, JsonVariant & json) {
        bool reboot = false;
        JsonDocument doc;
        doc = json.as<JsonObject>();
        int remPort = doc["alpRemPort"].as<int>();
        int alpacaPort = doc["alpPort"].as<int>();

        if (remPort > 500 && alpacaPort> 500){
            if(Config.alpacaPort.remotePort != remPort){
                reboot = true;
            }
            Config.alpacaPort.remotePort = remPort;
            if(Config.alpacaPort.alpacaPort != alpacaPort){
                reboot = true;
            }
            Config.alpacaPort.alpacaPort = alpacaPort;

            Config.save.board.execute = true;
            if (reboot){
                request->send(200, "application/json", "{\"reboot\": \"1\"}");
            } else {
                request->send(200, "application/json", "{\"accept\": \"1\"}");
            }
            return;
        }
        request->send(200, "application/json", "{\"error\": \"1\"}");
    });
    server.addHandler(boardcfg);
    
    server.serveStatic("/config.txt", SPIFFS, "/config.txt");
    server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico").setCacheControl("max-age=31536000");
    server.serveStatic("/assets/", SPIFFS, "/assets/").setCacheControl("max-age=31536000");

    server.onNotFound(notFound);
}

#endif