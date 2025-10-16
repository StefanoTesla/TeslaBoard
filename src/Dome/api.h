#ifndef DOME_WEBSERVER
#define DOME_WEBSERVER


void domeWebApi(){

    server.on("/api/dome/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        Dome.getConfiguration(doc);
        
        response->setLength();
        request->send(response);
    });

    server.on("/api/dome/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["enable"] = Dome.isEnable();

        JsonObject shutter = doc["shutter"].to<JsonObject>();
            shutter["roofState"] = Dome.shutter.getStatus();
            shutter["actualCommand"] = Dome.shutter.getActualCommand();

        JsonObject input = shutter["input"].to<JsonObject>();
            input["open"] = Dome.shutter.getOpenSensorRaw();
            input["close"] = Dome.shutter.getCloseSensorRaw();

        JsonObject autoclose = shutter["autoclose"].to<JsonObject>();
            autoclose["enable"] = Dome.shutter.isAutoCloseEnable();
            autoclose["remaing"] = Dome.shutter.autoCloseRemaningTime();

        response->setLength();
        request->send(response);
    }).addMiddleware(&isDomeEnable);

    server.on("/api/dome/open", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = false;

        if(Dome.shutter.isMoving())
        {
            doc["error"] = "ShAlreadyMoving";
            response->setLength();
            request->send(response);
            return;
        }

        if (Dome.shutter.isOpen()){
            doc["error"] = "ShAlreadyOpen";
            response->setLength();
            request->send(response);
            return;
        }
        
        doc["execute"] = true;
        Dome.shutter.open();
        
        response->setLength();
        request->send(response);
    }).addMiddleware(&isDomeEnable);

    server.on("/api/dome/close", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = false;

        if(Dome.shutter.isMoving())
        {
            doc["error"] = "ShAlreadyMoving";
            response->setLength();
            request->send(response);
            return;
        }

        if (Dome.shutter.isClosed()){
            doc["error"] = "ShAlreadyOpen";
            response->setLength();
            request->send(response);
            return;
        }

        doc["execute"] = true;
        Dome.shutter.close();

        response->setLength();
        request->send(response);
    }).addMiddleware(&isDomeEnable);

    server.on("/api/dome/halt", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = true;

        Dome.shutter.halt();
        response->setLength();
        request->send(response);
    }).addMiddleware(&isDomeEnable);

    AsyncCallbackJsonWebHandler* domeConfigHandler = new AsyncCallbackJsonWebHandler("/api/dome/cfg");

    domeConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
    domeConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& root) {
        
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        
        //convert the JsonVariant to JsonObject
        const JsonObject& incomingObj = root.as<JsonObject>();

        JsonArray err = doc["errors"].to<JsonArray>();

        Dome.validateConfiguration(incomingObj,doc);

        if(err.size()>0){
            response->setCode(500);
            response->setLength();
            request->send(response);
            return;
        }


        Dome.storeConfiguration(incomingObj);
        
        response->setLength();
        request->send(response);
    });

    server.addHandler(domeConfigHandler);


}
#endif