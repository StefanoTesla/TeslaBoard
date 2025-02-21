#ifndef CC_WEBSERVER
#define CC_WEBSERVER

void coverWebServer(){

    server.on("/api/coverc/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonObject calibrator = doc["calibrator"].to<JsonObject>();
        calibrator["present"] = CoverC.config.calibrator.present;
        calibrator["pin"] = Calibrator.getPinNumber();


        JsonObject cover = doc["cover"].to<JsonObject>();
        cover["present"] = CoverC.config.cover.present;
        cover["pin"] = Cover.getPinNumber();
        cover["closeDeg"] = Cover.closeDeg;
        cover["openDeg"] = Cover.openDeg;
        cover["maxDeg"] = Cover.getMax();
        cover["movTime"] = Cover.movingTime;
        
        doc["reboot"] = CoverC.config.save.restartNeeded;

        response->setLength();
        request->send(response);
    });

    server.on("/api/coverc/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonObject calibrator = doc["calibrator"].to<JsonObject>();
        calibrator["status"] = CoverC.status.calibrator.status;
        if(CoverC.config.calibrator.present){
            calibrator["brightness"] = CoverC.status.calibrator.actualBrightness;
        }

        JsonObject cover = doc["cover"].to<JsonObject>();
        cover["status"] = CoverC.status.cover.status;
        if(CoverC.config.cover.present){
            cover["angle"] = CoverC.status.cover.angle;
        }
        

        response->setLength();
        request->send(response);
    });
    server.on("/api/coverc/open", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        if(Cover.isMoving() == false && CoverC.config.cover.present){
            doc["execute"] = true;
            CoverC.command.cover.move = true;
            CoverC.command.cover.angle = Cover.openDeg;
        } else {
            doc["execute"] = false;
            if(!CoverC.config.cover.present){
                doc["error"] = "coverNotPresent";
            }
            if(Cover.isMoving()){
                doc["error"] = "coverIsMoving";
            }
        }

        response->setLength();
        request->send(response);
    });
    server.on("/api/coverc/close", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        if(Cover.isMoving() == false && CoverC.config.cover.present){
            doc["execute"] = true;
            CoverC.command.cover.move = true;
            CoverC.command.cover.angle = Cover.closeDeg;
        } else {
            doc["execute"] = false;
            if(!CoverC.config.cover.present){
                doc["error"] = "coverNotPresent";
            }
            if(Cover.isMoving()){
                doc["error"] = "coverIsMoving";
            }
        }

        response->setLength();
        request->send(response);
    });

    server.on("/api/coverc/brightness", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        String parameter;
        bool present = false;
        bool inRange = false;
        int value = 0;

        doc["execute"] = false;

        int paramsNr = request->params();
        if( CoverC.config.calibrator.present ){

            for (int i = 0; i < paramsNr; i++) {
                const AsyncWebParameter* p = request->getParam(i);
                parameter = p->name();
                if (parameter == "brightness") {
                    present = true;
                    value = p->value().toInt();
                    if(value >=0 && value <=4096){
                        CoverC.command.calibrator.change = true;
                        CoverC.command.calibrator.brightness = value;
                        inRange = true;
                        doc["execute"] = true;
                    }
                    else{
                        inRange = false;
                    }
                }  
            }
            if(!present){
                doc["error"] = "calibBrightnessNotPresent";
            } else {
                if(!inRange){
                    doc["error"] = "calibBrightnessNotInRange";
                }
            }
        } else {
            if(!present){
                doc["error"] = "calibratorNotPresent";
            }
        }

        response->setLength();
        request->send(response);
    });

    server.on("/api/coverc/on", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        doc["execute"] = false;

        if( CoverC.config.calibrator.present ){
            CoverC.command.calibrator.change = true;
            CoverC.command.calibrator.brightness = 4095;
            doc["execute"] = true;
        } else {
            doc["error"] = "calibratorNotPresent";
        }

        response->setLength();
        request->send(response);
    });

    server.on("/api/coverc/off", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        doc["execute"] = false;
        
        if( CoverC.config.calibrator.present ){
            CoverC.command.calibrator.change = true;
            CoverC.command.calibrator.brightness = 0;
            doc["execute"] = true;
        } else {
            doc["error"] = "calibratorNotPresent";
        }
        response->setLength();
        request->send(response);
    });


    AsyncCallbackJsonWebHandler* coverCConfigHandler = new AsyncCallbackJsonWebHandler("/api/coverc/cfg");

    coverCConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
    coverCConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& json) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            JsonArray err = doc["errors"].to<JsonArray>();
            bool error = false;
            bool reboot = false;
            
            /* calibrator */
            JsonObject calibrator = json.as<JsonObject>()["calibrator"];
            if( calibrator["present"].is<bool>()){
                if(calibrator["present"] != CoverC.config.calibrator.present){
                    reboot = true;
                }
            } else {
                error=true;
                err.add("Calibrator present");
            }
            if( calibrator["pin"].is<unsigned int>() && commonValidateOutputPin(calibrator["pin"])){
                if (calibrator["pin"] != Calibrator.getPinNumber()){
                    reboot = true;
                }
            } else {
                error=true;
                err.add("GPIO Calibrator pin");
            }

            /* cover */
            JsonObject cover = json.as<JsonObject>()["cover"];
            if( cover["present"].is<bool>()){
                if(cover["present"] != CoverC.config.cover.present){
                    reboot = true;
                }
            } else {
                error=true;
                err.add("Cover enable");
            }
            if( cover["pin"].is<unsigned int>() and commonValidateOutputPin(cover["pin"])){
                if (cover["pin"] != Cover.getPinNumber()){
                    reboot = true;
                }
            } else {
                error=true;
                err.add("GPIO Cover pin");
            }
            if( !cover["maxDeg"].is<unsigned int>() || ( cover["openDeg"] > 360 || cover["closeDeg"] > 360)){
                error=true;
                err.add("Cover MaxDeg");
            }
            if( !cover["closeDeg"].is<unsigned int>() || cover["closeDeg"] > 360){
                error=true;
                err.add("Close Cover deg");
            }
            if( !cover["openDeg"].is<unsigned int>() || cover["openDeg"] > 360){
                error=true;
                err.add("Open Cover deg");
            }
            if( !cover["movTime"].is<unsigned int>()){
                error=true;
                err.add("mov time out");
            }

            if(!error){
                CoverCConfigTmp = json;
                CoverC.config.save.execute = true;

                if(!reboot ){
                    Cover.openDeg = cover["openDeg"];
                    Cover.closeDeg = cover["closeDeg"];
                    Cover.setMax(cover["maxDeg"]);
                    Cover.movingTime = cover["movTime"];
                }
            } else {
                response->setCode(500);
            }
            doc["reboot"] = reboot;
            CoverC.config.save.restartNeeded = reboot;

            response->setLength();
            request->send(response);
        });

    server.addHandler(coverCConfigHandler);

    server.serveStatic("/coverc/ccconfig.txt", LittleFS, "/cfg/cccfg.txt");
}

#endif