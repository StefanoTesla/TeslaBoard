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
                jsonSwitch["maxDeg"] = servo->maxDeg;
                jsonSwitch["movingTime"] = servo->movingTime;
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
            logMessageFormatted(Switches,lErr,"cmd. not exec, ID out of range",id);
            doc["error"] = "SwIdOutOfRange";
            err= true;
        }
        if(SwitchObjects[id] == nullptr){
            logMessageFormatted(Switches,lErr,"cmd. not exec, ID is nullPointer",id);
            doc["error"] = "SwIdOutOfRange";
            err= true;
        }
        if(value < 0){
            doc["error"] = "SwValueAbsent";
            logMessageFormatted(Switches,lErr,"cmd. not exec on ID %d value not provided",id);
            err= true;
        } else {
            if(value < Switch.data[id].property.minValue){
                doc["error"] = "SwValueBehindMin";
                logMessageFormatted(Switches,lErr,"cmd. not exec on ID %d with val: %d below min",id,value);
                err= true;
            }
            if(value > Switch.data[id].property.maxValue){
                doc["error"] = "SwValueOverMax";
                logMessageFormatted(Switches,lErr,"cmd. not exec on ID %d with val: %d exceeded max",id,value);
                err= true;
            }
        }

        switch (Switch.data[id].property.type)
        {
        case SwTypeDInput:
        case SwTypeNull:
            err = true;
            doc["error"] = "SwNotWritable";
            logMessageFormatted(Switches,lErr,"cmd. not exec on ID %d switch cannot be writable",id);
            err= true;
            break;
        
        default:
            break;
        }


        if(!err){
            Switch.data[id].command.intValue = value;
            if(value == Switch.data[id].property.minValue){
                Switch.data[id].command.boValue = false;
            } else {
                Switch.data[id].command.boValue = true;
            }
            Switch.data[id].command.execute = true;
            doc["execute"] = true;
        }

        response->setLength();
        request->send(response);
    });

    AsyncCallbackJsonWebHandler* switchConfigHandler = new AsyncCallbackJsonWebHandler("/api/switch/cfg");

    switchConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
    switchConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& json) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            JsonArray err = doc["errors"].to<JsonArray>();

            bool error = false;
            bool reboot = false;
            int count = 0;
            unsigned int type = 0;
            logMessageFormatted(Switches,lInfo,"New incoming config");

            //clear the temporary configration and rebuild the structure
            tmpSwitchCfg.clear();
            JsonArray IncomingSwitch = tmpSwitchCfg["Switches"].to<JsonArray>();
            for (JsonObject Switche : json["Switches"].as<JsonArray>()) {

                //check if a name is provided
                if(!Switche["name"].is<String>()){
                    error = true;
                    err.add("Name not provided");
                    logMessageFormatted(Switches,lErr,"Name not provided - id: %d",count);
                    continue;
                }
                //check if a desc is provided
                if(!Switche["desc"].is<String>()){
                    error = true;
                    err.add("Desc not provided");
                    logMessageFormatted(Switches,lErr,"Description not provided - id: %d",count);
                    continue;
                }

                //check the IO type
                if(!Switche["type"].is<unsigned int>()){
                    error = true;
                    err.add("Type not passed");
                    logMessageFormatted(Switches,lErr,"Type not provided - id: %d",count);
                    continue;
                }

                if(Switche["type"].as<unsigned int>() < 0 || Switche["type"].as<unsigned int>()>4){
                    error = true;
                    err.add("Type not defined");
                    logMessageFormatted(Switches,lErr,"undefinded GPIO Type - id: %d",count);
                    continue;
                }

                type = Switche["type"].as<unsigned int>();

                if(type == 0){
                    logMessageFormatted(Switches,lErr,"Switch of type 0, skipped - id: %d",count);
                    continue;
                }


                //check if pin is a number
                if(!Switche["pin"].is<unsigned int>()){
                    error = true;
                    err.add("Pin number not provided");
                    logMessageFormatted(Switches,lErr,"Pin number not provided - id: %d",count);
                    continue;
                }

                if(count>= _MAX_SWITCH_ID_){
                    error = true;
                    err.add("More than possibile Switch Configured");
                    logMessageFormatted(Switches,lErr,"More than possibile Switch Configured - id: %d",count);
                    continue;
                }

                //Digital Input
                if(type == 1){
                    //pin, name, type, invert, don, doff
                    if(!pinUsableAsInput(Switche["pin"].as<unsigned int>())){
                        error = true;
                        err.add("pin can't be used as input");
                        logMessageFormatted(Switches,lErr,"Pin can't be used as input - id: %d",count);
                        continue;
                    }
                    JsonObject tmpSwitch = IncomingSwitch.add<JsonObject>();

                    tmpSwitch["name"] = Switche["name"];
                    tmpSwitch["desc"] = Switche["desc"];
                    tmpSwitch["type"] = 1;
                    tmpSwitch["pin"] = Switche["pin"].as<unsigned int>();
                    tmpSwitch["invert"] = 0;
                    tmpSwitch["dOn"] = 0;
                    tmpSwitch["dOff"] = 0;

                    if(Switche["invert"].as<unsigned int>() >= 0 && Switche["invert"].as<unsigned int>() <= 1){
                        tmpSwitch["invert"] = Switche["invert"].as<unsigned int>();
                    }

                    if(Switche["dOn"].as<unsigned int>() >= 0){
                        tmpSwitch["dOn"] = Switche["dOn"].as<unsigned int>();
                    }

                    if(Switche["dOff"].as<unsigned int>() >= 0){
                        tmpSwitch["dOff"] = Switche["dOff"].as<unsigned int>();
                    }

                } else if(type==2){
                    if(!pinUsableAsOutput(Switche["pin"].as<unsigned int>())){
                        error = true;
                        err.add("pin can't be used as input");
                        logMessageFormatted(Switches,lErr,"Pin can't be used as input - id: %d",count);
                        continue;
                    }
                    JsonObject tmpSwitch = IncomingSwitch.add<JsonObject>();
                    tmpSwitch["name"] = Switche["name"];
                    tmpSwitch["desc"] = Switche["desc"];
                    tmpSwitch["type"] = 2;
                    tmpSwitch["pin"] = Switche["pin"].as<unsigned int>();
                    tmpSwitch["invert"] = 0;
                    
                    if(Switche["invert"].as<unsigned int>() >= 0 && Switche["invert"].as<unsigned int>() <= 1){
                        tmpSwitch["invert"] = Switche["invert"].as<unsigned int>();
                    }
                } else if(type==3){
                    if(!pinUsableAsOutput(Switche["pin"].as<unsigned int>())){
                        error = true;
                        err.add("pin can't be used as input");
                        logMessageFormatted(Switches,lErr,"Pin can't be used as input - id: %d",count);
                        continue;
                    }
                    JsonObject tmpSwitch = IncomingSwitch.add<JsonObject>();
                    tmpSwitch["name"] = Switche["name"];
                    tmpSwitch["desc"] = Switche["desc"];
                    tmpSwitch["type"] = 3;
                    tmpSwitch["pin"] = Switche["pin"].as<unsigned int>();
                } else if(type==4){
                    if(!pinUsableAsOutput(Switche["pin"].as<unsigned int>())){
                        error = true;
                        err.add("pin can't be used as input");
                        logMessageFormatted(Switches,lErr,"Pin can't be used as input - id: %d",count);
                        continue;
                    }
                    JsonObject tmpSwitch = IncomingSwitch.add<JsonObject>();
                    tmpSwitch["name"] = Switche["name"];
                    tmpSwitch["desc"] = Switche["desc"];
                    tmpSwitch["type"] = 4;
                    tmpSwitch["pin"] = Switche["pin"].as<unsigned int>();
                    tmpSwitch["openDeg"] = 0;
                    tmpSwitch["closeDeg"] = 0;
                    tmpSwitch["movTime"] = 0;

                    if(Switche["maxDeg"].as<unsigned int>() >= 0 && Switche["maxDeg"].as<unsigned int>() <= 360){
                        tmpSwitch["maxDeg"] = Switche["maxDeg"].as<unsigned int>();
                    }
                    if(Switche["openDeg"].as<unsigned int>() >= 0 && Switche["openDeg"].as<unsigned int>() <= Switche["maxDeg"].as<unsigned int>()){
                        tmpSwitch["openDeg"] = Switche["openDeg"].as<unsigned int>();
                    }
                    if(Switche["closeDeg"].as<unsigned int>() >= 0 && Switche["closeDeg"].as<unsigned int>() <= Switche["maxDeg"].as<unsigned int>()){
                        tmpSwitch["closeDeg"] = Switche["invert"].as<unsigned int>();
                    }
                    if(Switche["movTime"].as<unsigned int>() >= 0){
                        tmpSwitch["movTime"] = Switche["movTime"].as<unsigned int>();
                    }
                } else {
                    continue;
                }
            }


            doc["reboot"] = reboot;

            if(!reboot){
                //reassign data
            }

            if(!error){
                logMessage(Switches,lInfo,"Config don't have any errors, I'm going to store it");
                Switch.config.save.execute = true;
            } else {
                logMessage(Switches,lErr,"Config got errors, I'm NOT going to store it");
                response->setCode(500);
            }

            response->setLength();
            request->send(response);
        });

    server.addHandler(switchConfigHandler);

    server.serveStatic("/switch/switchconfig.txt", LittleFS, "/cfg/switchcfg.txt");
}

#endif