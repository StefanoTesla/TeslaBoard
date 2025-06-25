#ifndef SWITCH_WEBSERVER
#define SWITCH_WEBSERVER


void switchWebServer(){

    server.on("/api/switch/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonArray array = doc["Switches"].to<JsonArray>();

        for(int i=0; i < Switch.config.configuredSwitch; i++){
            if (SwitchObjects[i] == nullptr) { continue; }
            JsonObject jsonSwitch = array.add<JsonObject>();
            jsonSwitch["name"] = SwitchObjects[i]->getName();
            jsonSwitch["desc"] = SwitchObjects[i]->getDescription();
            jsonSwitch["type"] = SwitchObjects[i]->getType(); 
            jsonSwitch["pin"] = SwitchObjects[i]->getPinNumber();
            // uncommon parameters
            
            //digital input
            if(SwitchObjects[i]->getType() == SwTypeDInput){
                //digital input
                DigitalInput* input = static_cast<DigitalInput*>(SwitchObjects[i]);
                jsonSwitch["dOn"] = input->dOn;
                jsonSwitch["dOff"] = input->dOff;
                jsonSwitch["invert"] = input->invert;
                
            } else if(SwitchObjects[i]->getType() == SwTypeDOutput){
                //digital output
                DigitalOutput* output = static_cast<DigitalOutput*>(SwitchObjects[i]);
                jsonSwitch["invert"] = output->invert;

            } else if (SwitchObjects[i]->getType() == SwTypeServo){
                //servo
                ServoOutput* servo = static_cast<ServoOutput*>(SwitchObjects[i]);
                jsonSwitch["openDeg"] = servo->openDeg;
                jsonSwitch["closeDeg"] = servo->closeDeg;
                jsonSwitch["maxDeg"] = servo->getMax();
                jsonSwitch["movTime"] = servo->movingTime;
            }

        }
        response->setLength();
        request->send(response);
    });

    server.on("/api/switch/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        JsonArray array = doc["Switches"].to<JsonArray>();
        for(int i=0; i< Switch.config.configuredSwitch; i++){
            if (SwitchObjects[i] == nullptr) {
                continue;
            } 
            JsonObject jsonSwitch = array.add<JsonObject>();
            jsonSwitch["name"] = SwitchObjects[i]->getName();
            jsonSwitch["desc"] = SwitchObjects[i]->getDescription();
            jsonSwitch["type"] = SwitchObjects[i]->getType(); 
            jsonSwitch["min"] = SwitchObjects[i]->getMin(); 
            jsonSwitch["max"] = SwitchObjects[i]->getMax();
            jsonSwitch["boValue"] = SwitchObjects[i]->status() ? true : false;
            jsonSwitch["intValue"] = SwitchObjects[i]->status();
        }
        
        response->setLength();
        request->send(response);
    });

    server.on("/api/switch/set-value", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        String parameter;
        doc["execute"] = false;
        bool err = false;
        bool exist = false;
        bool inRange = false;
        int id = -1;
        int value = -1;
        int paramsNr = request->params();
        
        for (int i = 0; i < paramsNr; i++) {
            const AsyncWebParameter* p = request->getParam(i);
            parameter = p->name();
            if (parameter == "id") {
                id = p->value().toInt();
            } 
            if (parameter == "value") {
                value = p->value().toInt();
            } 
        }
        if(id < 0 || id >= Switch.config.configuredSwitch){
            doc["error"] = "SwIdOutOfRange";
            err= true;
        }
        if(SwitchObjects[id] == nullptr){
            doc["error"] = "SwIdOutOfRange";
            err= true;
        }
        if(value < 0){
            doc["error"] = "SwValueAbsent";
            err= true;
        } else {
            if(value < SwitchObjects[id]->getMin()){
                doc["error"] = "SwValueBehindMin";
                err= true;
            }
            if(value > SwitchObjects[id]->getMax()){
                doc["error"] = "SwValueOverMax";
                err= true;
            }
        }

        if (SwitchObjects[id]->getType() < static_cast<int>(SwTypeDOutput))
        {
            err = true;
            doc["error"] = "SwNotWritable";
        }


        if(err){
            response->setLength();
            request->send(response);
            return;
        }

        if (SwitchObjects[id]->getType() != static_cast<int>(SwTypeServo)){
            SwitchObjects[id]->write(value);
        } else {
            ServoOutput* servo = static_cast<ServoOutput*>(SwitchObjects[id]);
            servo->goTo(value,true);
        }
        
        doc["execute"] = true;
        

        response->setLength();
        request->send(response);
    });

    AsyncCallbackJsonWebHandler* switchConfigHandler = new AsyncCallbackJsonWebHandler("/api/switch/cfg");

    switchConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
    switchConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& root) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        JsonArray err = doc["errors"].to<JsonArray>();

        bool docError = false;
        bool validError = false;
        int retVal = 0;

        bool reboot = false;
        int count = 0;
        unsigned int type = 0;

        /* check json structure */
        if (!root["Switches"].is<JsonArray>()) {
                docError = true;
                err.add("Switches data doesn't exist");
            }

        for (JsonVariant element : root["Switches"].as<JsonArray>()) {
                if (!element.is<JsonObject>()) {
                    docError = true;
                    err.add("Switch malformed json");
                    break; // oppure break, o gestisci come vuoi
                }
                count += 1;
            }

        if(count>=_MAX_SWITCH_ID_){
                docError = true;
                err.add("More than possibile Switch Configured");
            }

        if (docError){
                response->setCode(500);
                response->setLength();
                request->send(response);
            return;
        }


        //clear the temporary configration and rebuild the structure
        tmpSwitchCfg.clear();
        JsonArray IncomingSwitch = tmpSwitchCfg["Switches"].to<JsonArray>();

        /* validate data and store them*/
        /* to avoid a new loop olso reboot check is performed */
        count = 0;
        for (JsonObject Switche : root["Switches"].as<JsonArray>()) {

            unsigned int type = 0;
            type = Switche["type"].as<unsigned int>();
            if(type == 0){ continue; }

            //check if a name is provided
            if(!Switche["name"].is<String>()){
                validError = true;
                JsonObject e = err.add<JsonObject>();
                e["id"] = count;
                e["error"] = "IONameUndef";
                continue;
            }
            //check if a desc is provided
            if(!Switche["desc"].is<String>()){
                validError = true;
                JsonObject e = err.add<JsonObject>();
                e["id"] = count;
                e["error"] = "IODescUndef";
                continue;
            }
            //check the IO type
            if(!Switche["type"].is<unsigned int>()){
                validError = true;
                JsonObject e = err.add<JsonObject>();
                e["id"] = count;
                e["error"] = "IOTypeUndef";
                continue;
            }

            type = Switche["type"].as<unsigned int>();
            Serial.println(type);
            if(type>4){
                validError = true;
                JsonObject e = err.add<JsonObject>();
                e["id"] = count;
                e["error"] = "IOTypeOutRange";
                continue;
            }
                
            retVal = 0;
            //Digital Input
            if(type== static_cast<int>(SwTypeDInput)){
                if(validateJsonInput(Switche) !=1){
                    JsonObject e = err.add<JsonObject>();
                    e["id"] = count;
                    e["error"] = retValTranslate(retVal);
                    continue;
                }
                JsonObject tmpSwitch = IncomingSwitch.add<JsonObject>();
                tmpSwitch["name"] = Switche["name"];
                tmpSwitch["desc"] = Switche["desc"];
                tmpSwitch["type"] = 1;
                copyInputJson(Switche,tmpSwitch);
            
            } else if (type == static_cast<int>(SwTypeDOutput)){
                if(validateJsonOutput(Switche) !=1){
                    validError = true;
                    JsonObject e = err.add<JsonObject>();
                    e["id"] = count;
                    e["error"] = retValTranslate(retVal);
                    continue;
                }
                JsonObject tmpSwitch = IncomingSwitch.add<JsonObject>();
                tmpSwitch["name"] = Switche["name"];
                tmpSwitch["desc"] = Switche["desc"];
                tmpSwitch["type"] = 2;
                copyOutputJson(Switche,tmpSwitch);

            } else if (type == static_cast<int>(SwTypePWM)){
                if(validateJsonPwm(Switche) !=1){
                    validError = true;
                    JsonObject e = err.add<JsonObject>();
                    e["id"] = count;
                    e["error"] = retValTranslate(retVal);
                    continue;
                }
                JsonObject tmpSwitch = IncomingSwitch.add<JsonObject>();
                tmpSwitch["name"] = Switche["name"];
                tmpSwitch["desc"] = Switche["desc"];
                tmpSwitch["type"] = 3;
                copyPWMJson(Switche,tmpSwitch);

            } else if(type == static_cast<int>(SwTypeServo)){
                if(validateJsonServo(Switche) !=1){
                    validError = true;
                    JsonObject e = err.add<JsonObject>();
                    e["id"] = count;
                    e["error"] = retValTranslate(retVal);
                    continue;
                }
                JsonObject tmpSwitch = IncomingSwitch.add<JsonObject>();
                tmpSwitch["name"] = Switche["name"];
                tmpSwitch["desc"] = Switche["desc"];
                tmpSwitch["type"] = 4;
                copyServoJson(Switche,tmpSwitch);
            }
        }

        if(validError){
            response->setCode(500);
            response->setLength();
            request->send(response);
            return;
        }

        Serial.println();
        /* check if I need to reboot 
        If the GPIO under analysis don't need reboot we store new data directly
        */
        int incomingType;
        
        for (int i = 0; i < _MAX_SWITCH_ID_; i++)
        {
            incomingType = IncomingSwitch[i]["type"].as<unsigned int>();
            // if switch is not configured and 
            if(SwitchObjects[i] == nullptr){
                //1
                //incoming is null don't need nothing
                if(incomingType == 0){ 
                    continue;
                } else {
                    //2
                    //incoming is defined required a startup process
                    reboot = true;
                    continue;
                }
            }
            
            //since now switch is configured.

            //3
            // deleted by incoming data require a startup process
            if(incomingType == 0){
                reboot = true;
                continue;
            }
            //4
            // switch is configured but incoming type is different require a startup process
            if(SwitchObjects[i]->getType() != incomingType){
                reboot = true;
                continue;
            } else {
                //from now switch and incoming type is equal
                //5
                //pin is different
                if(SwitchObjects[i]->getPinNumber() != IncomingSwitch[i]["pin"].as<unsigned int>()){
                    reboot = true;
                    continue;
                }

                //type is checked in two step before, pin is checked in the step before
                //now we should pass data don't need reboot if reboot 

                if(SwitchObjects[i]->getType() == static_cast<int>(SwTypeDInput)){
                    DigitalInput* di = static_cast<DigitalInput*>(SwitchObjects[i]);
                    di->dOn = IncomingSwitch[i]["dOn"].as<unsigned long>();
                    di->dOff = IncomingSwitch[i]["dOff"].as<unsigned long>();
                    di->invert = IncomingSwitch[i]["invert"].as<unsigned int>();
                }
                if(SwitchObjects[i]->getType() == static_cast<int>(SwTypeDOutput)){
                    DigitalOutput* out = static_cast<DigitalOutput*>(SwitchObjects[i]);
                    out->invert = IncomingSwitch[i]["invert"].as<unsigned int>();
                }
                //pwm switch got only pin
                if(SwitchObjects[i]->getType() == static_cast<int>(SwTypeServo)){
                    ServoOutput* servo = static_cast<ServoOutput*>(SwitchObjects[i]);
                    servo->setMax(IncomingSwitch[i]["maxDeg"].as<unsigned int>());
                    servo->openDeg = IncomingSwitch[i]["openDeg"].as<unsigned int>();
                    servo->closeDeg = IncomingSwitch[i]["closeDeg"].as<unsigned int>();
                    servo->movingTime = IncomingSwitch[i]["movTime"].as<unsigned int>();
                }
            }
        }        

        doc["reboot"] = reboot;
        Switch.config.save.restartNeeded = reboot;
        Switch.config.save.execute = true;
        response->setLength();
        request->send(response);
        });

    server.addHandler(switchConfigHandler);

    server.serveStatic("/switch/switchcfg.txt", LittleFS, "/cfg/switchcfg.txt");
}

#endif