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
    domeConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& json) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        JsonArray err = doc["errors"].to<JsonArray>();
        bool error = false;
        bool reboot = false;
        
        /* open pin */
        JsonObject pinOpen = json.as<JsonObject>()["pinOpen"];
        if( pinOpen["pin"].is<unsigned int>() and commonValidateInputPin(pinOpen["pin"])){
            if (pinOpen["pin"] != DomeInOpen.getPinNumber()){
                reboot = true;
            }
        } else {
            error=true;
            err.add("GPIO Open Input");
        }

        if( !pinOpen["dOn"].is<unsigned long>() ){
            error=true;
            err.add("Open Input delay ON error");
        }

        if( !pinOpen["dOff"].is<unsigned long>()){
            error=true;
            err.add("Open Input delay OFF error");
        }

        if( !pinOpen["invert"].is<int>()){
            error=true;
            err.add("Open Input type");
        } else if(pinOpen["invert"] < 0 || pinOpen["invert"] > 1){
            error=true;
            err.add("Open pin Type error");
        }

        /* close pin */
        JsonObject pinClose = json.as<JsonObject>()["pinClose"];

        if( pinClose["pin"].is<unsigned int>() and commonValidateInputPin(pinClose["pin"])){
            if (pinClose["pin"] != DomeInClose.getPinNumber()){
                reboot = true;
            }
        } else {
            error=true;
            err.add("GPIO Close Input");
        }
        if( !pinClose["dOn"].is<unsigned int>() ){
            error=true;
            err.add("Open Close delay ON error");
        }
        if( !pinClose["dOff"].is<unsigned int>()){
            error=true;
            err.add("Open Close delay OFF error");
        }
        if( !pinClose["invert"].is<int>()){
            error=true;
            err.add("Close pin Type error");
        } else if(pinClose["invert"] < 0 || pinClose["invert"] > 1){
            error=true;
            err.add("Close pin Type error");
        }
        


        /* outputs */

        /* start pin */
        JsonObject pinStart = json.as<JsonObject>()["pinStart"];
        if( pinStart["pin"].is<unsigned int>() and commonValidateOutputPin(pinStart["pin"])){
            if (pinStart["pin"] != DomeOutMoveOpen.getPinNumber()){
                reboot = true;
            }
        } else {
            error=true;
            err.add("GPIO Start Output");
        }
    
        if( !pinStart["invert"].is<int>()){
            error=true;
            err.add("GPIO Start Output type");
        } else if(pinStart["invert"] < 0 || pinStart["invert"] > 1){
            error=true;
            err.add("GPIO Start Output Type error");
        }

        

        JsonObject pinHalt = json.as<JsonObject>()["pinHalt"];
        if( pinHalt["pin"].is<unsigned int>() and commonValidateOutputPin(pinHalt["pin"])){
            if (pinHalt["pin"] != DomeOutHaltClose.getPinNumber()){
                reboot = true;
            }
        } else {
            error=true;
            err.add("GPIO HALT Output");
        }

        if( !pinHalt["invert"].is<int>()){
            error=true;
            err.add("GPIO Start Output type");
        } else if(pinHalt["invert"] < 0 || pinHalt["invert"] > 1){
            error=true;
            err.add("GPIO Start Output Type error");
        }

        /* timeout */
        if( !json["movTimeOut"].is<unsigned int>()){
            error=true;
            err.add("Move Time Out");
        }

        /* auto close*/
        JsonObject autoClose = json.as<JsonObject>()["autoclose"];
        if( !autoClose["enable"].is<bool>()){
            error=true;
            err.add("Enable Auto Close");
        }
        if( !autoClose["minutes"].is<unsigned int>()){
            error=true;
            err.add("Minutes for Auto Close");
        }

        if(!error){
            /* input open */
            DomeConfigTmp.clear();
            DomeConfigTmp = json;
            
            /* apply data that don't require reboot */
            DomeInOpen.invert = pinOpen["invert"].as<int>();
            DomeInOpen.dOn = pinOpen["dOn"].as<unsigned long>();
            DomeInOpen.dOff = pinOpen["dOff"].as<unsigned long>();

            DomeInClose.invert = pinClose["invert"].as<int>();
            DomeInClose.dOn = pinClose["dOn"].as<unsigned long>();
            DomeInClose.dOff = pinClose["dOff"].as<unsigned long>();

            DomeOutMoveOpen.invert = pinStart["invert"].as<int>();
            DomeOutHaltClose.invert = pinHalt["invert"].as<int>();

            /* timeout */
            Dome.config.data.movingTimeOut = json["movTimeOut"].as<unsigned int>();
            
            /* autoclose */
            Dome.config.data.enAutoClose = autoClose["enable"].as<bool>();
            Dome.config.data.autoCloseTimeOut = autoClose["minutes"].as<unsigned int>();
            Dome.config.Save.execute = true;
        } else {
            response->setCode(500);
        }
        doc["reboot"] = reboot;
        Dome.config.Save.restartNeeded = reboot;

        response->setLength();
        request->send(response);
    });

    server.addHandler(domeConfigHandler);

    server.serveStatic("/dome/domeconfig.txt", LittleFS, "/cfg/domecfg.txt");

;
}
#endif