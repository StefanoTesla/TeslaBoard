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

            serializeJson(root, Serial);
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
        for (JsonObject Switche : root["Switches"].as<JsonArray>()) {

            //check if a name is provided
            if(!Switche["name"].is<String>()){
                validError = true;
                JsonObject e = err.add<JsonObject>();
                e["id"] = count;
                e["error"] = "Name not provided";
                continue;
            }
            //check if a desc is provided
            if(!Switche["desc"].is<String>()){
                validError = true;
                JsonObject e = err.add<JsonObject>();
                e["id"] = count;
                e["error"] = "Description not provided";
                continue;
            }
            //check the IO type
            if(!Switche["type"].is<unsigned int>()){
                validError = true;
                JsonObject e = err.add<JsonObject>();
                e["id"] = count;
                e["error"] = "Type not provided";
                continue;
            }
            unsigned int type = 0;
            type = Switche["type"].as<unsigned int>();

            if(type>4){
                validError = true;
                JsonObject e = err.add<JsonObject>();
                e["id"] = count;
                e["error"] = "Type out of range";
                continue;
            }

            if(type == 0){ continue; }

            if(!Switche["pin"].is<unsigned int>()){
                validError = true;
                JsonObject e = err.add<JsonObject>();
                e["id"] = count;
                e["errore"] = "Pin not provided";
                continue;
            }
                
            retVal = 0;
            //Digital Input
            if(type== static_cast<int>(SwTypeDInput)){
                retVal=validateJsonInput(Switche);
                if (retVal !=1){
                    JsonObject e = err.add<JsonObject>();
                    e["id"] = count;
                    e["error"] = retValTranslate(retVal);
                    continue;
                }
                JsonObject tmpSwitch = IncomingSwitch.add<JsonObject>();
                tmpSwitch["name"] = Switche["name"];
                tmpSwitch["desc"] = Switche["desc"];
                tmpSwitch["type"] = 1;
                tmpSwitch["pin"] = Switche["pin"].as<unsigned int>();
                tmpSwitch["invert"] = Switche["invert"].as<unsigned int>();
                tmpSwitch["dOn"] = Switche["dOn"].as<unsigned int>();
                tmpSwitch["dOff"] = Switche["dOff"].as<unsigned int>();
            
            } else if (type == static_cast<int>(SwTypeDOutput)){
                retVal=validateJsonOutput(Switche);
                if (retVal !=1){
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
                tmpSwitch["pin"] = Switche["pin"].as<unsigned int>();
                tmpSwitch["invert"] = Switche["invert"].as<unsigned int>();

            } else if (type == static_cast<int>(SwTypePWM)){
                retVal=validateJsonOutput(Switche);
                if (retVal !=1){
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
                tmpSwitch["pin"] = Switche["pin"].as<unsigned int>();

            } else if(type == static_cast<int>(SwTypeServo)){
                retVal=validateJsonServo(Switche);
                if (retVal !=1){
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
                tmpSwitch["pin"] = Switche["pin"].as<unsigned int>();
                tmpSwitch["maxDeg"] = Switche["maxDeg"].as<unsigned int>();
                tmpSwitch["openDeg"] = Switche["openDeg"].as<unsigned int>();
                tmpSwitch["closeDeg"] = Switche["closeDeg"].as<unsigned int>();
            }
        }

        if(validError){
            response->setCode(500);
            response->setLength();
            request->send(response);
            return;
        }
        //* Check if I need to reboot



        doc["reboot"] = reboot;

        if(!reboot){
            //reassign data
        }

        if(!validError){
            Switch.config.save.execute = true;
        } else {
            response->setCode(500);
        }

        response->setLength();
        request->send(response);
        });

    server.addHandler(switchConfigHandler);

    server.serveStatic("/switch/switchconfig.txt", LittleFS, "/cfg/switchcfg.txt");
}

#endif