#ifndef SWITCH_WEBSERVER
#define SWITCH_WEBSERVER
#include "libraries.h"
#include "header.h"

void switchWebServer(){

    server.on("/api/switch/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonArray array = doc["Switches"].to<JsonArray>();

        for(int i=0;i<_MAX_SWITCH_ID_;i++){
            if(Switch.data[i].property.type == SwTypeNull  ){
                continue;
            }
            JsonObject jsonSwitch = array.add<JsonObject>();
            jsonSwitch["name"] = Switch.data[i].property.Name;
            jsonSwitch["desc"] = Switch.data[i].property.Description;
            jsonSwitch["type"] = Switch.data[i].property.type;
            jsonSwitch["pin"] = Switch.data[i].property.pin;
            jsonSwitch["min"] = Switch.data[i].property.minValue;
            jsonSwitch["max"] = Switch.data[i].property.maxValue;

        }
        
        response->setLength();
        request->send(response);
    });

    server.on("/api/switch/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonArray array = doc["Switches"].to<JsonArray>();

        for(int i=0;i<_MAX_SWITCH_ID_;i++){
            Serial.println(SwitchObjects[i]->getType());
            /*
            if(SwitchObjects[i]->getType() == SwTypeNull  ){
                continue;
            }
            if (SwitchObjects[i] != nullptr) {
            JsonObject jsonSwitch = array.add<JsonObject>();
            jsonSwitch["name"] = SwitchObjects[i]->getName();
            jsonSwitch["desc"] = SwitchObjects[i]->getDescription();
            jsonSwitch["type"] = SwitchObjects[i]->getType();
            jsonSwitch["min"] = SwitchObjects[i].property.minValue;
            jsonSwitch["max"] = SwitchObjects[i].property.maxValue;
            if( Switch.data[i].actualValue.boValue ){
                jsonSwitch["boValue"] = true;   
            } else {
                jsonSwitch["boValue"] = false;   
            }
            
            jsonSwitch["intValue"] = Switch.data[i].actualValue.intValue;
            } else {
                Serial.print("Puntatore nullo id: ");
                Serial.println(i);
            }*/
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
        int value = 0;
        int paramsNr = request->params();
        
        for (int i = 0; i < paramsNr; i++) {
            const AsyncWebParameter* p = request->getParam(i);
            parameter = p->name();
            if (parameter == "id") {
                id = p->value().toInt();
            } 
            if (parameter == "value") {
                exist = true;
                value = p->value().toInt();
            } 
        }
        if(id < 0 || id >= _MAX_SWITCH_ID_){
            logMessageFormatted(Switches,lErr,"cmd. not exec, ID out of range",id);
            doc["error"] = "SwIdOutOfRange";
            err= true;
        }
        if(exist){
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
        } else {
            doc["error"] = "SwValueAbsent";
            logMessageFormatted(Switches,lErr,"cmd. not exec on ID %d value not provided",id);
            err= true;
        }

        switch (Switch.data[id].property.type)
        {
        case SwTypeAInput:
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
            for (JsonObject Switche : json["Switches"].as<JsonArray>()) {
                SwitchProperty propertyStruct;

                if(!Switche["name"].is<String>() || !Switche["type"].is<unsigned int>() || !Switche["pin"].is<unsigned int>()){
                    err.add("Name, type or pin not defined");
                    logMessageFormatted(Switches,lErr,"Name, type or pin not defined");
                    continue;
                }

                type = Switche["type"].as<unsigned int>();

                if(type == 0 || type > 6){
                    err.add("Switch of type 0 skipped");
                    logMessage(Switches,lErr,"Switch of type 0, skipped");
                    continue;
                }
                propertyStruct.type = static_cast<SwitchType>(type);

                if (propertyStruct.type == SwTypeDInput){
                    Serial.println("New Input");
                    propertyStruct.pin = Switche["pin"].as<unsigned int>();
                    if(!pinUsableAsInput(propertyStruct.pin)){
                        err.add("pin not usable as input");
                        logMessage(Switches,lErr,"pin not usable as input");
                        continue;
                    }
                    propertyStruct.minValue = 0;
                    propertyStruct.maxValue = 1;   
                } else if ( propertyStruct.type == SwTypeDOutput ) {
                    Serial.println("New Output");
                    propertyStruct.pin = Switche["pin"].as<unsigned int>();
                    if(!pinUsableAsOutput(propertyStruct.pin)){
                        err.add("pin not usable as output");
                        logMessage(Switches,lErr,"pin not usable as output");
                        continue;
                    }
                    propertyStruct.minValue = 0;
                    propertyStruct.maxValue = 1; 
                } else if ( propertyStruct.type == SwTypePWM ) {
                    Serial.println("New PWM");
                    propertyStruct.pin = Switche["pin"].as<unsigned int>();
                    if(!pinUsableAsOutput(propertyStruct.pin)){
                        err.add("pin not usable as pwm output");
                        logMessage(Switches,lErr,"pin not usable as pwm");
                        continue;
                    }
                    propertyStruct.minValue = 0;
                    propertyStruct.maxValue = 4096;   
                } else if ( propertyStruct.type == SwTypeServo ) {
                    Serial.println("New Servo");
                    propertyStruct.pin = Switche["pin"].as<unsigned int>();
                    if(!pinUsableAsOutput(propertyStruct.pin)){
                        err.add("pin not usable as servo output");
                        logMessage(Switches,lErr,"pin not usable as servo");
                        continue;
                    }    
                    if(!Switche["min"].is<unsigned int>() || !Switche["max"].is<unsigned int>()){
                        err.add("Min or Max value not defined");
                        logMessage(Switches,lErr,"Min or Max value not defined");
                        continue;
                    }
                    propertyStruct.minValue = Switche["min"];
                    propertyStruct.maxValue = Switche["max"];

                } else {
                    
                    Serial.println("not implemented");
                    logMessage(Switches,lErr,"Type not implemented");
                    continue;
                }

                const char* jsonName = Switche["name"].as<const char*>();
                strncpy(propertyStruct.Name, jsonName, sizeof(propertyStruct.Name) - 1);
                propertyStruct.Name[sizeof(propertyStruct.Name) - 1] = '\0';

                const char* jsonDesc = Switche["desc"].as<const char*>();
                strncpy(propertyStruct.Description, jsonDesc, sizeof(propertyStruct.Description) - 1);
                propertyStruct.Description[sizeof(propertyStruct.Description) - 1] = '\0';
                Switch.config.tmp[count].property = propertyStruct;
                count +=1;
            }

            //do I need to reboot?
            for (int i = 0; i < _MAX_SWITCH_ID_; i++)
            {
                if(Switch.data[i].property.type != Switch.config.tmp[i].property.type ||
                    Switch.data[i].property.pin != Switch.config.tmp[i].property.pin)
                {
                    reboot = true;
                }
            }
            
            doc["reboot"] = reboot;

            if(!reboot){
                for (int i = 0; i < _MAX_SWITCH_ID_; i++)
                {
                    /*just name, desc, min and max*/
                    strncpy(Switch.data[i].property.Name, Switch.config.tmp[i].property.Name, sizeof(Switch.data[i].property.Name) - 1);
                    Switch.data[i].property.Name[sizeof(Switch.data[i].property.Name) - 1] = '\0';
                    strncpy(Switch.data[i].property.Description, Switch.config.tmp[i].property.Description, sizeof(Switch.data[i].property.Description) - 1);
                    Switch.data[i].property.Description[sizeof(Switch.data[i].property.Description) - 1] = '\0';
                    Switch.data[i].property.minValue = Switch.config.tmp[i].property.minValue;
                    Switch.data[i].property.maxValue = Switch.config.tmp[i].property.maxValue;
                }
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