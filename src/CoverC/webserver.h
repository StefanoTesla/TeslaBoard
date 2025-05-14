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
                    if(value >=0 && value <=4095){
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
    coverCConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& root) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        JsonArray err = doc["errors"].to<JsonArray>();

        bool docError = false;
        bool validError = false;
        int retVal = 0;
        bool reboot = false;
        
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
        JsonObject calibrator = root.as<JsonObject>()["calibrator"];
        JsonObject cover = root.as<JsonObject>()["cover"];


        /* data validation */
        if(!calibrator["present"].is<bool>()){
            validError = true;
            err.add("CoverC: calibrator present wrong data tpye");
        }

        bool calibPresent = calibrator["present"].as<bool>();

        if(calibPresent){
           retVal = validateJsonPwm(calibrator["pwm"]);

           if(retVal !=1){
            validError = true;
            switch (retVal)
            {
            case -1:
                err.add("CoverC: calibrator pin wrong data tpye");
                break;
            case -10:
                err.add("CoverC: calibrator pin not usable as output");
                break;
            
            default:
                break;
            }
           }
        }

        if(!cover["present"].is<bool>()){
            validError = true;
            err.add("CoverC: calibrator present wrong data tpye");
        }

        bool coverPresent = cover["present"].as<bool>();

        if(coverPresent){
            retVal = validateJsonServo(calibrator["pwm"]);

           if(retVal !=1){
            validError = true;
            switch (retVal)
            {
            case -1:
                err.add("CoverC: cover pin wrong data type");
                break;
            case -10:
                err.add("CoverC: cover pin not usable as output");
                break;
            case -2:
                err.add("CoverC: cover maxDeg wrong data tpye");
                break;
            case -200:
                err.add("CoverC: cover maxDeg is out of range");
                break;
            case -3:
                err.add("CoverC: cover openDeg wrong data type");
                break;
            case -300:
                err.add("CoverC: cover openDeg is out of range");
                break;
            case -301:
                err.add("CoverC: cover openDeg is bigger than maxDeg");
                break;
            case -4:
                err.add("CoverC: cover closeDeg wrong data type");
                break;
            case -400:
                err.add("CoverC: cover closeDeg is out of range");
                break;
            case -401:
                err.add("CoverC: cover closeDeg is bigger than maxDeg");
                break;
            case -5:
                err.add("CoverC: cover movTime wrong data type");
                break;            
            default:
                break;
            }
           }
        }

        if (validError){
            response->setCode(500);
            response->setLength();
            request->send(response);
            return;
        }

        /* check if module need reboot */

        if(calibPresent != CoverC.config.calibrator.present){
            reboot = true;
        }

        //if it was present and I want it present,check the pin
        if(calibPresent && CoverC.config.calibrator.present){
            if(calibrator["pwm"]["pin"].as<unsigned int>() != Calibrator.getPinNumber()){
                reboot = true;
            }
        }


        if(coverPresent != CoverC.config.cover.present){
            reboot = true;
        }

        if(coverPresent && CoverC.config.cover.present){
            if(cover["servo"]["pin"].as<unsigned int>() != Cover.getPinNumber()){
                reboot = true;
            }
        }

        /* store data that don't need setup */

        Cover.openDeg = cover["openDeg"];
        Cover.closeDeg = cover["closeDeg"];
        Cover.setMax(cover["maxDeg"]);
        Cover.movingTime = cover["movTime"];

        CoverCConfigTmp.clear();
        CoverCConfigTmp = reboot;
        doc["reboot"] = reboot;
        CoverC.config.save.restartNeeded = reboot;
        CoverC.config.save.execute = true;
        
        response->setLength();
        request->send(response);
        
        });

    server.addHandler(coverCConfigHandler);

    server.serveStatic("/coverc/ccconfig.txt", LittleFS, "/cfg/cccfg.txt");
}

#endif