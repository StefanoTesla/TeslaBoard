#ifndef CC_WEBSERVER
#define CC_WEBSERVER

void coverWebServer(){

    server.on("/api/coverc/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonObject calibrator = doc["calibrator"].to<JsonObject>();
        calibrator["present"] = CoverC.config.calibrator.present;
        calibrator["pwm"]["pin"] = Calibrator.getPinNumber();

        JsonObject cover = doc["cover"].to<JsonObject>();
        cover["present"] = CoverC.config.cover.present;
        
        JsonObject servo = cover["servo"].to<JsonObject>();
        servo["pin"] = Cover.getPinNumber();
        servo["closeDeg"] = Cover.closeDeg;
        servo["openDeg"] = Cover.openDeg;
        servo["maxDeg"] = Cover.getMax();
        servo["movTime"] = Cover.movingTime;
        
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
            calibrator["brightness"] = Calibrator.status();
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
                    if(value >=0 && value <=Calibrator.getMax()){
                        Calibrator.write(value);
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
            Calibrator.write(Calibrator.getMax());
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
            Calibrator.write(0);
            doc["execute"] = true;
        } else {
            doc["error"] = "calibratorNotPresent";
        }
        response->setLength();
        request->send(response);
    });


    AsyncCallbackJsonWebHandler* coverCConfigHandler = new AsyncCallbackJsonWebHandler("/api/coverc/cfg");

    coverCConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
    coverCConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& root) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        JsonArray err = doc["errors"].to<JsonArray>();

        JsonObject servo = JsonObject();
        bool docError = false;
        bool validError = false;
        int retVal = 0;
        bool reboot = false;
        CoverCConfigTmp.clear();

        /* check json structure */
        if (!root["calibrator"].is<JsonObject>()) {
            docError = true;
            err.add("Calibrator data doesn't exist");
        }
        if (!root["cover"].is<JsonObject>()) {
            docError = true;
            err.add("Cover data doesn't exist");
        }

        if (docError){
            response->setCode(500);
            response->setLength();
            request->send(response);
            return;
        }  

        /* peek the objects */
        JsonObject inCalibrator = root.as<JsonObject>()["calibrator"];
        JsonObject inCover = root.as<JsonObject>()["cover"];


        /* data validation */
        if(!inCalibrator["present"].is<bool>()){
            validError = true;
            err.add("CoverC: calibrator present wrong data tpye");
        } else {
    
        }

        bool calibPresent = inCalibrator["present"].as<bool>();

        if(calibPresent){
           retVal = validateJsonPwm(inCalibrator["pwm"]);

           if(retVal !=1){
            validError = true;
            JsonObject e = err.add<JsonObject>();
            e["id"] = 1;
            e["error"] = retValTranslate(retVal);
           }
        }

        if(!inCover["present"].is<bool>()){
            validError = true;
            err.add("CoverC: calibrator present wrong data tpye");
        }

        bool coverPresent = inCover["present"].as<bool>();

        if(coverPresent){
            retVal = validateJsonServo(inCover["servo"]);

           if(retVal !=1){
            validError = true;
            JsonObject e = err.add<JsonObject>();
            e["id"] = 2;
            e["error"] = retValTranslate(retVal);
           }
        }

        if (validError){
            response->setCode(500);
            response->setLength();
            request->send(response);
            return;
        }

        /* creating the new file */
        /*
        I don't know if is the most beutiful way, the old way was to copy the json coming from the request,
        but if I send large json with unused keys them was stored...
        */
        JsonObject newDoc = CoverCConfigTmp.to<JsonObject>(); // prepara l'oggetto JSON
        JsonObject calib = newDoc["calibrator"].to<JsonObject>();

        calib["present"] = calibPresent;
        if(calibPresent){
            JsonObject pwm = calib["pwm"].to<JsonObject>();
            copyPWMJson(inCalibrator["pwm"],pwm);
        }

        JsonObject cover = newDoc["cover"].to<JsonObject>();
        cover["present"] = coverPresent;

        if(coverPresent){
            servo = cover["servo"].to<JsonObject>();
            copyServoJson(inCover["servo"],servo);
        }

        /* check if module need reboot */

        if(calibPresent != CoverC.config.calibrator.present){
            reboot = true;
        }

        //if it was present and I want it present,check the pin
        if(calibPresent && CoverC.config.calibrator.present){
            if(inCalibrator["pwm"]["pin"].as<unsigned int>() != Calibrator.getPinNumber()){
                reboot = true;
            }
        }


        if(coverPresent != CoverC.config.cover.present){
            reboot = true;
        } else if(coverPresent && CoverC.config.cover.present){
            if(servo["pin"].as<unsigned int>() != Cover.getPinNumber()){
                reboot = true;
            }
        }

        /* store data that don't need setup */
        if(!reboot && coverPresent){
            Cover.openDeg = servo["openDeg"].as<unsigned int>();
            Cover.closeDeg = servo["closeDeg"].as<unsigned int>();
            Cover.setMax(servo["maxDeg"].as<unsigned int>());
            Cover.movingTime = servo["movTime"].as<unsigned int>();
        }
        
        doc["reboot"] = reboot;
        CoverC.config.save.restartNeeded = reboot;
        CoverC.config.save.execute = true;
        
        response->setLength();
        request->send(response);
        
        });

    server.addHandler(coverCConfigHandler);

}

#endif