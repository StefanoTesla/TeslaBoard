#ifndef BOARD_WEBSERVER
#define BOARD_WEBSERVER


void boardWebServer(){

    server.on("/api/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["locale"] = Global.config.language.locale;

        JsonArray modules = doc["modules"].to<JsonArray>();
        JsonObject dome = modules.add<JsonObject>();
        dome["name"] = "dome";
        dome["enable"] = Dome.config.isEnable;
        dome["order"] = Dome.config.order;
        JsonObject sw = modules.add<JsonObject>();
        sw["name"] = "switch";
        sw["enable"] = Switch.config.isEnable;
        sw["order"] = Switch.config.order;
        JsonObject coverc = modules.add<JsonObject>();
        coverc["name"] = "coverc";
        coverc["enable"] = CoverC.config.isEnable;
        coverc["order"] = CoverC.config.order;
        

        doc["version"] = SW_VERSION;
        response->setLength();
        request->send(response);
    });


    server.on("/api/board/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["locale"] = Global.config.language.locale;

        JsonObject wifi = doc["wifi"].to<JsonObject>();
        wifi["reconTime"] = Global.config.wifi.reconnection.intervall;
        wifi["actualip"] = WiFi.localIP();
        wifi["mac"] = WiFi.macAddress();

        JsonObject address = doc["address"].to<JsonObject>();
        address["enStaticIP"] = Global.config.wifi.ip.enable;

        JsonObject ip = address["staticIp"].to<JsonObject>();
        ip["0"]= Global.config.wifi.ip.ip[0];
        ip["1"]= Global.config.wifi.ip.ip[1];
        ip["2"]= Global.config.wifi.ip.ip[2];
        ip["3"]= Global.config.wifi.ip.ip[3];

        JsonObject gw = address["staticGateway"].to<JsonObject>();
        gw["0"]= Global.config.wifi.ip.gw[0];
        gw["1"]= Global.config.wifi.ip.gw[1];
        gw["2"]= Global.config.wifi.ip.gw[2];
        gw["3"]= Global.config.wifi.ip.gw[3];


        JsonObject sub = address["staticSubnet"].to<JsonObject>();
        sub["0"]= Global.config.wifi.ip.sub[0];
        sub["1"]= Global.config.wifi.ip.sub[1];
        sub["2"]= Global.config.wifi.ip.sub[2];
        sub["3"]= Global.config.wifi.ip.sub[3];

        response->setLength();
        request->send(response);
    });


    server.on("/api/board/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonObject wifi = doc["wifi"].to<JsonObject>();
        wifi["uptime"] = Global.config.wifi.upTime.minutes;
        wifi["ssid"] = WiFi.SSID();
        wifi["db"] = WiFi.RSSI();
        wifi["ip"] = WiFi.localIP();
        wifi["sub"] = WiFi.subnetMask();
        wifi["mac"] = WiFi.macAddress();

        JsonObject memo = doc["memory"].to<JsonObject>();
        memo["heapSize"] = ESP.getHeapSize();
        memo["freeHeap"] = ESP.getFreeHeap();
        memo["minHeap"] = ESP.getMinFreeHeap();

        JsonObject board = doc["board"].to<JsonObject>();
        board["uptime"] = Global.config.esp32.upTime.minutes;
        board["cpu"] = ESP.getChipModel();
        board["cores"] = ESP.getChipCores();
        board["rev"] = ESP.getChipRevision();
        board["freq"] = ESP.getCpuFreqMHz();
        
        response->setLength();
        request->send(response);
    });


    AsyncCallbackJsonWebHandler* boardConfigHandler = new AsyncCallbackJsonWebHandler("/api/board/cfg");

    boardConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
    boardConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& json) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            JsonArray err = doc["errors"].to<JsonArray>();
            bool error = false;
            bool reboot = false;
            bool enable = false;
            
            serializeJson(json, Serial);

            if(!json["locale"].is<String>()){
                error = true;
                doc["errors"].add("Wrong Locale");
            }
            if(!json["wifi"]["reconTime"].is<unsigned int>()){
                error = true;
                doc["errors"].add("Wrong reconnection time");
            }

            if(!error){
            Global.config.save.execute = true;
            }

            if(error){
                response->setCode(500);
            }
            response->setLength();
            request->send(response);
        });

    server.addHandler(boardConfigHandler);


    server.on("/api/board/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = "See you next time...";
        
        response->setLength();
        request->send(response);
        Global.config.reboot.rebootRequest =true;
    });

}

#endif