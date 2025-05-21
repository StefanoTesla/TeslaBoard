#ifndef DOME_WEBSERVER
#define DOME_WEBSERVER

void domeWebServer(){

    server.on("/api/dome/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonObject pinOpen = doc["pinOpen"].to<JsonObject>();
        pinOpen["pin"] = DomeInOpen.getPinNumber();
        pinOpen["dOn"] = DomeInOpen.dOn;
        pinOpen["dOff"] = DomeInOpen.dOff;
        pinOpen["invert"] = DomeInOpen.invert;

        JsonObject pinClose = doc["pinClose"].to<JsonObject>();
        pinClose["pin"] = DomeInClose.getPinNumber();
        pinClose["dOn"] = DomeInClose.dOn;
        pinClose["dOff"] = DomeInClose.dOff;
        pinClose["invert"] = DomeInClose.invert;

        JsonObject autoclose = doc["autoclose"].to<JsonObject>();
        autoclose["enable"] = Dome.config.data.enAutoClose;
        autoclose["minutes"] = Dome.config.data.autoCloseTimeOut;

        JsonObject pinStart = doc["pinStart"].to<JsonObject>();
        pinStart["pin"] = DomeOutMoveOpen.getPinNumber();
        pinStart["invert"] = DomeOutMoveOpen.invert;

        JsonObject pinHalt = doc["pinHalt"].to<JsonObject>();
        pinHalt["pin"] = DomeOutHaltClose.getPinNumber();
        pinHalt["invert"] = DomeOutHaltClose.invert;

        doc["movTimeOut"] = Dome.config.data.movingTimeOut;
        doc["driverType"] = Dome.config.data.driverType;
        doc["reboot"] = Dome.config.Save.restartNeeded;
        response->setLength();
        request->send(response);
    });

    server.on("/api/dome/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonObject status = doc["status"].to<JsonObject>();
        status["roofState"] = Dome.Shutter.status;
        status["actualCommand"] = Dome.Shutter.command;
        status["lastCommand"] = Dome.Shutter.LastDomeCommand;

        JsonObject input = doc["input"].to<JsonObject>();
        input["open"] = DomeInOpen.status();
        input["close"] = DomeInClose.status();

        response->setLength();
        request->send(response);
    });

    server.on("/api/dome/open", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = false;

        if(Dome.Shutter.command != ShCommandIdle)
        {
            doc["error"] = "ShAlreadyMoving";
            response->setLength();
            request->send(response);
            return;
        }

        if (Dome.Shutter.status != ShStatusOpen){
            doc["error"] = "ShAlreadyOpen";
            response->setLength();
            request->send(response);
            return;
        }
        
        doc["execute"] = true;
        Dome.Shutter.command = ShCommandOpen;
        
        response->setLength();
        request->send(response);
    });

    server.on("/api/dome/close", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = false;

        if(Dome.Shutter.command == ShCommandIdle && Dome.Shutter.status != ShStatusClose){
            doc["execute"] = true;
            Dome.Shutter.command = ShCommandClose;

        } else {
            if(Dome.Shutter.status == ShStatusClose){
                doc["error"] = "ShAlreadyClosed";
            } else {
                doc["error"] = "ShAlreadyMoving";
            }
        }

        response->setLength();
        request->send(response);
    });

    server.on("/api/dome/halt", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = true;

        Dome.Shutter.command = ShCommandHalt;
        response->setLength();
        request->send(response);
    });

    AsyncCallbackJsonWebHandler* domeConfigHandler = new AsyncCallbackJsonWebHandler("/api/dome/cfg");

    domeConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
    domeConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& root) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        JsonArray err = doc["errors"].to<JsonArray>();
        bool docError = false;
        bool validError = false;
        int retVal = 0;
        bool reboot = false;

        /* check json structure */

        if (!root["pinOpen"].is<JsonObject>()) {
            docError = true;
            err.add("Open Input data doesn't exist");
        }
        if (!root["pinClose"].is<JsonObject>() && !docError) {
            docError = true;
            err.add("Close Input data doesn't exist");
        }
        if (!root["pinStart"].is<JsonObject>() && !docError) {
            docError = true;
            err.add("Start/Open data doesn't exist");
        }
        if (!root["pinHalt"].is<JsonObject>() && !docError) {
            docError = true;
            err.add("Halt/Close data doesn't exist");
        }
        if (!root["autoclose"].is<JsonObject>() && !docError) {
            docError = true;
            err.add("Open Input data doesn't exist");
        }
        if (!root["movTimeOut"].is<unsigned int>() && !docError) {
            docError = true;
            err.add("Moving timeout data doesn't exist or wrong data type");
        }
        
        if (docError){
            response->setCode(500);
            response->setLength();
            request->send(response);
            return;
        }
        /* peek the objects */
        JsonObject pinOpen = root.as<JsonObject>()["pinOpen"];
        JsonObject pinClose = root.as<JsonObject>()["pinClose"];
        JsonObject pinStart = root.as<JsonObject>()["pinStart"];
        JsonObject pinHalt = root.as<JsonObject>()["pinHalt"];
        JsonObject autoClose = root.as<JsonObject>()["autoclose"];

        /* data validation */
        retVal = validateJsonInput(pinOpen);
        if(retVal != 1){
            validError = true;
            JsonObject e = err.add<JsonObject>();
            e["id"] = 1;
            e["error"] = retValTranslate(retVal);
        }
        retVal=0;
        retVal = validateJsonInput(pinClose);
        if(retVal != 1){
            validError = true;
            JsonObject e = err.add<JsonObject>();
            e["id"] = 2;
            e["error"] = retValTranslate(retVal);
        }
        retVal=0;
        retVal = validateJsonOutput(pinStart);
        if(retVal != 1){
            validError = true;
            JsonObject e = err.add<JsonObject>();
            e["id"] = 3;
            e["error"] = retValTranslate(retVal);
        }
        retVal=0;
        retVal = validateJsonOutput(pinHalt);
        if(retVal != 1){
            validError = true;
            JsonObject e = err.add<JsonObject>();
            e["id"] = 4;
            e["error"] = retValTranslate(retVal);
        }

        if( !autoClose["enable"].is<bool>()){
            validError=true;
            err.add("Auto Close: enable wrong data type");
        }
        if( !autoClose["minutes"].is<unsigned int>()){
            validError=true;
            err.add("Auto Close:  minutes wrong data type");
        }

        if (validError){
            response->setCode(500);
            response->setLength();
            request->send(response);
            return;
        }

        /* check if module need reboot */
        if (pinOpen["pin"].as<unsigned int>() != DomeInOpen.getPinNumber()
        || pinClose["pin"].as<unsigned int>() != DomeInClose.getPinNumber()
        || pinStart["pin"].as<unsigned int>() != DomeOutMoveOpen.getPinNumber()
        || pinHalt["pin"].as<unsigned int>() != DomeOutHaltClose.getPinNumber()){
            reboot = true;
        }


        DomeConfigTmp.clear();
        DomeConfigTmp = root;
        
        if(!reboot){
        DomeInOpen.invert = pinOpen["invert"].as<int>();
        DomeInOpen.dOn = pinOpen["dOn"].as<unsigned long>();
        DomeInOpen.dOff = pinOpen["dOff"].as<unsigned long>();

        DomeInClose.invert = pinClose["invert"].as<int>();
        DomeInClose.dOn = pinClose["dOn"].as<unsigned long>();
        DomeInClose.dOff = pinClose["dOff"].as<unsigned long>();

        DomeOutMoveOpen.invert = pinStart["invert"].as<int>();
        DomeOutHaltClose.invert = pinHalt["invert"].as<int>();

        Dome.config.data.movingTimeOut = root["movTimeOut"].as<unsigned int>();
            
        Dome.config.data.enAutoClose = autoClose["enable"].as<bool>();
        Dome.config.data.autoCloseTimeOut = autoClose["minutes"].as<unsigned int>();
        }

        Dome.config.Save.execute = true;
        Dome.config.Save.restartNeeded = reboot;
        doc["reboot"] = reboot;

        response->setLength();
        request->send(response);
    });

    server.addHandler(domeConfigHandler);
    server.serveStatic("/dome/domeconfig.txt", LittleFS, "/cfg/domecfg.txt");

;
}
#endif