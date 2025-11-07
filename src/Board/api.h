#ifndef BOARD_WEBSERVER
#define BOARD_WEBSERVER

extern BoardModule Board;
extern DomeModule Dome;
extern CoverCalibratorModule CoverCalibrator;
extern SwitchModule Switches;


void boardWebServer(){

    server.on("/api/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["locale"] = Board.getLocale();

        JsonArray modules = doc["modules"].to<JsonArray>();
        JsonObject dome = modules.add<JsonObject>();
        dome["name"] = "dome";
        dome["enable"] = Dome.isEnable();
        dome["order"] = Dome.uiOrder;
        JsonObject sw = modules.add<JsonObject>();
        sw["name"] = "switch";
        sw["enable"] = Switches.isEnable();
        sw["order"] = Switches.uiOrder;
        JsonObject coverc = modules.add<JsonObject>();
        coverc["name"] = "coverc";
        coverc["enable"] = CoverCalibrator.isEnable();
        coverc["order"] = CoverCalibrator.uiOrder;
        

        doc["version"] = SW_VERSION;
        response->setLength();
        request->send(response);
    });

    server.on("/api/board/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["locale"] = Board.getLocale();

        JsonObject wifi = doc["wifi"].to<JsonObject>();
        wifi["actualip"] = WiFi.localIP();
        wifi["mac"] = WiFi.macAddress();

        response->setLength();
        request->send(response);
    });

    server.on("/api/board/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonObject wifi = doc["wifi"].to<JsonObject>();
        wifi["uptime"] = 0;
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
        board["uptime"] = Board.getUptime();
        board["cpu"] = ESP.getChipModel();
        board["cores"] = ESP.getChipCores();
        board["rev"] = ESP.getChipRevision();
        board["freq"] = ESP.getCpuFreqMHz();
        
        response->setLength();
        request->send(response);
    });


    AsyncCallbackJsonWebHandler* boardConfigHandler = new AsyncCallbackJsonWebHandler("/api/board/cfg");

    boardConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
    boardConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& root) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        const JsonObject& incomingObj = root.as<JsonObject>();

        JsonArray err = doc["errors"].to<JsonArray>();

        CoverCalibrator.validateConfiguration(incomingObj,doc);

        if(err.size()>0){
            response->setCode(500);
            response->setLength();
            request->send(response);
            return;
        }


        CoverCalibrator.storeConfiguration(incomingObj);
        });

    server.addHandler(boardConfigHandler);


    server.on("/api/board/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = "See you next time...";
        
        response->setLength();
        request->send(response);

    });

}

#endif