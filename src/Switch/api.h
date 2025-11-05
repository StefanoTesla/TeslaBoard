#ifndef SWITCH_WEBSERVER
#define SWITCH_WEBSERVER

extern SwitchModule Switches;

#pragma region ErrorsHandler


/* id realted error messages*/

void missingIdErrorMessage(AsyncWebServerRequest * request) {
    AsyncJsonResponse * response = prepareAlpacaResponse(request);
    JsonObject doc = response -> getRoot();

    doc["ErrorNumber"] = 1025;
    doc["ErrorMessage"] = "ID not provided";

    response -> setLength();
    response -> setCode(400);
    request -> send(response);
}

void IdOutOfRangeErrorMessage(AsyncWebServerRequest * request) {
    AsyncJsonResponse * response = prepareAlpacaResponse(request);
    JsonObject doc = response -> getRoot();
    char message[100];
    int maxID = Switches.getConfiguredSwitch();
    int id = request -> getAttribute("id").toInt();
    sprintf(message, "ID provided: %d, outside range, maximum is: %d - 1", id,
        maxID);
    doc["ErrorNumber"] = 1025;
    doc["ErrorMessage"] = message;

    response -> setLength();
    response -> setCode(400);
    request -> send(response);
}

void UnconfiguredIdErrorMessage(AsyncWebServerRequest * request) {
    AsyncJsonResponse * response = prepareAlpacaResponse(request);
    JsonObject doc = response -> getRoot();
    char message[100];
    int maxID = Switches.getConfiguredSwitch();
    int id = request -> getAttribute("id").toInt();
    sprintf(message, "The switch with ID: %d was not configured.", id);
    doc["ErrorNumber"] = 1025;
    doc["ErrorMessage"] = message;

    response -> setLength();
    response -> setCode(400);
    request -> send(response);
}

void unWritableIdErrorMessage(AsyncWebServerRequest * request) {
    AsyncJsonResponse * response = prepareAlpacaResponse(request);
    JsonObject doc = response -> getRoot().to < JsonObject > ();
    char message[100];
    int id = request -> getAttribute("id").toInt();
    sprintf(message, "Switch n: %d, cannot be written", id);
    doc["ErrorNumber"] = 1025;
    doc["ErrorMessage"] = message;
    response -> setLength();
    response -> setCode(400);
    request -> send(response);
}

void missingStateErrorMessage(AsyncWebServerRequest * request) {
    AsyncJsonResponse * response = prepareAlpacaResponse(request);
    JsonObject doc = response -> getRoot().to < JsonObject > ();
    char message[100];
    sprintf(message, "\"State\" parameter not provided");
    doc["ErrorNumber"] = 1025;
    doc["ErrorMessage"] = message;
    response -> setLength();
    response -> setCode(400);
    request -> send(response);
}

void missingValueErrorMessage(AsyncWebServerRequest * request) {
    AsyncJsonResponse * response = prepareAlpacaResponse(request);
    JsonObject doc = response -> getRoot();

    doc["ErrorNumber"] = 1025;
    doc["ErrorMessage"] = "\"Value\" parameter not provided";

    response -> setLength();
    response -> setCode(400);
    request -> send(response);
}

void valueIsAboveMinErrorMessage(AsyncWebServerRequest * request) {
    AsyncJsonResponse * response = prepareAlpacaResponse(request);
    JsonObject doc = response -> getRoot();
    char message[100];
    int id = request -> getAttribute("id").toInt();
    int value = request -> getAttribute("value").toInt();
    sprintf(message, "Switch id: %d, the value %d is below minimum allowed value", id, value);
    doc["ErrorNumber"] = 1025;
    doc["ErrorMessage"] = message;

    response -> setLength();
    response -> setCode(400);
    request -> send(response);
}

void valueIsGreaterMaxErrorMessage(AsyncWebServerRequest * request) {
    AsyncJsonResponse * response = prepareAlpacaResponse(request);
    JsonObject doc = response -> getRoot();
    char message[100];
    int id = request -> getAttribute("id").toInt();
    int value = request -> getAttribute("value").toInt();
    sprintf(message, "Switch id: %d, the value %d is greater maximum allowed value", id, value);
    doc["ErrorNumber"] = 1025;
    doc["ErrorMessage"] = message;

    response -> setLength();
    response -> setCode(400);
    request -> send(response);
}
#pragma endregion

#pragma region Middleware

AsyncMiddlewareFunction modEN([](AsyncWebServerRequest * request, ArMiddlewareNext next) {
    if (Switches.isEnable()) {
        next();
    } else {
        request -> send(403, "text/plain", "Forbidden");
    }
});

AsyncMiddlewareFunction checkID([](AsyncWebServerRequest * request, ArMiddlewareNext next) {
    String idP = request -> getAttribute("id", String("null"));

    if (idP == "null") {
        missingIdErrorMessage(request);
        return;
    }

    int id = idP.toInt();
    
    switch (Switches.isValidID(id))
    {
    case -1:
        IdOutOfRangeErrorMessage(request);
        return;
    case -2:
        UnconfiguredIdErrorMessage(request);
        return;
    }

    next();
});

AsyncMiddlewareFunction checkIdAndValue([](AsyncWebServerRequest * request,ArMiddlewareNext next) {
    String idP = request -> getAttribute("id", String("null"));

    if (idP == "null") {
        missingIdErrorMessage(request);
        return;
    }
    int id = idP.toInt();

    String valueP = request -> getAttribute("value", String("null"));
    if (valueP == "null") {
        missingStateErrorMessage(request);
        return;
    }

    int value = valueP.toInt();

    switch (Switches.isValidValue(id,value))
    {
    case -1:
        IdOutOfRangeErrorMessage(request);
        return;
    case -2:
        UnconfiguredIdErrorMessage(request);
        return;
    case -3:
        unWritableIdErrorMessage(request);
        return;
    case -4:
        valueIsAboveMinErrorMessage(request);
        return;
    case -5:
        valueIsAboveMinErrorMessage(request);
        return;
    }
    
    next();
});

AsyncMiddlewareFunction checkIdAndState([](AsyncWebServerRequest * request,ArMiddlewareNext next) {

    String idP = request -> getAttribute("id", String("null"));
    if (idP == "null") {
        missingIdErrorMessage(request);
        return;
    }
    int id = idP.toInt();

    switch (Switches.isWritable(id))
    {
    case -1:
        IdOutOfRangeErrorMessage(request);
        return;
    case -2:
        UnconfiguredIdErrorMessage(request);
        return;
    case -3:
        unWritableIdErrorMessage(request);
        return;
    }
    
    String state = request -> getAttribute("state", String("null"));
    if (state == "null") {
        missingStateErrorMessage(request);
        return;
    } 
    
    
    next();
    
});


#pragma endregion

void webApi() {

    #pragma region webApi

    server.on("/api/switch/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = new AsyncJsonResponse();
        JsonObject doc = response -> getRoot().to < JsonObject > ();
        doc["enable"] = Switches.isEnable();
        doc["order"] = Switches.uiOrder;
        doc["identifier"] = Switches.getIdentifier();

        Switches.getConfiguration(doc);

        response -> setLength();
        request -> send(response);
    });

    server.on("/api/switch/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = new AsyncJsonResponse();
        JsonObject doc = response -> getRoot().to < JsonObject > ();
        JsonArray array = doc["Switches"].to < JsonArray > ();

        for (size_t i = 0; i < Switches.getConfiguredSwitch(); i++) {
            JsonObject swi = array.add < JsonObject > ();
            Switches.reportSwitchState(i, swi);
        }

        response -> setLength();
        request -> send(response);
    });

    server.on("/api/switch/set-value", HTTP_POST,
        [](AsyncWebServerRequest * request) {
            AsyncJsonResponse * response = new AsyncJsonResponse();
            JsonObject doc = response -> getRoot().to < JsonObject > ();
            String parameter;
            doc["execute"] = false;
            bool inRange = false;
            int id = -1;
            int value = -1;
            int paramsNr = request -> params();

            for (int i = 0; i < paramsNr; i++) {
                const AsyncWebParameter * p = request -> getParam(i);
                parameter = p -> name();
                if (parameter == "id") {
                    id = p -> value().toInt();
                }
                if (parameter == "value") {
                    value = p -> value().toInt();
                }
            }

            if (id == -1) {
                doc["error"] = "SwIdOAbsent";
                response -> setLength();
                request -> send(response);
                return;
            }

            if (value == -1) {
                doc["error"] = "SwValueAbsent";
                response -> setLength();
                request -> send(response);
                return;
            }

            switch (Switches.setSwitchValue(id, value)) {
            case 1:
                doc["execute"] = true;
                break;
            case -1:
                doc["error"] = "SwIdOutOfRange";
                break;
            case -2:
                doc["error"] = "SwIdNotConfigured";
                break;
            case -3:
                doc["error"] = "SwNotWritable";
                break;
            case -4:
                doc["error"] = "SwValueBehindMin";
                break;
            case -5:
                doc["error"] = "SwValueOverMax";
                break;

            default:
                doc["error"] = "Undefined Error";
                break;
            }
            response -> setLength();
            request -> send(response);
        });

    AsyncCallbackJsonWebHandler * switchConfigHandler = new AsyncCallbackJsonWebHandler("/api/switch/cfg");

    switchConfigHandler -> setMethod(HTTP_POST | HTTP_PUT);
    switchConfigHandler -> onRequest([](AsyncWebServerRequest * request, JsonVariant & root) {
        AsyncJsonResponse * response = new AsyncJsonResponse();
        JsonObject doc = response -> getRoot().to < JsonObject > ();

        // convert the JsonVariant to JsonObject
        const JsonObject & incomingObj = root.as < JsonObject > ();

        JsonArray err = doc["errors"].to < JsonArray > ();

        Switches.validateConfiguration(incomingObj, doc);

        if (err.size() > 0) {
            response -> setCode(500);
            response -> setLength();
            request -> send(response);
            return;
        }

        Switches.storeConfiguration();

        response -> setLength();
        request -> send(response);
    });

    server.addHandler(switchConfigHandler);

    #pragma endregion

    #pragma region AlpachaManagement

    alpaca.on("/api/v1/switch/0/name", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        doc["Value"] = Switches.getIdentifier() + " - TeslaBoard";

        response -> setLength();
        request -> send(response);
    }).addMiddleware( & getAlpParams);

    alpaca
        .on("/api/v1/switch/0/description", HTTP_GET,
            [](AsyncWebServerRequest * request) {
                AsyncJsonResponse * response = prepareAlpacaResponse(request);
                JsonObject doc = response -> getRoot();

                doc["Value"] = "Switch handled by Stefano TeslaBoard";

                response -> setLength();
                request -> send(response);
            })
        .addMiddleware( & getAlpParams);

    alpaca.on("/api/v1/switch/0/driverversion", HTTP_GET,
        [](AsyncWebServerRequest * request) {
            AsyncJsonResponse * response = prepareAlpacaResponse(request);
            JsonObject doc = response -> getRoot();

            doc["Value"] = "4.0.0";

            response -> setLength();
            request -> send(response);
        });

    alpaca
        .on("/api/v1/switch/0/driverinfo", HTTP_GET,
            [](AsyncWebServerRequest * request) {
                AsyncJsonResponse * response = prepareAlpacaResponse(request);
                JsonObject doc = response -> getRoot();

                doc["Value"] = "Servo can be async now";

                response -> setLength();
                request -> send(response);
            })
        .addMiddleware( & getAlpParams);

    alpaca
        .on("/api/v1/switch/0/interfaceversion", HTTP_GET,
            [](AsyncWebServerRequest * request) {
                AsyncJsonResponse * response = prepareAlpacaResponse(request);
                JsonObject doc = response -> getRoot();

                doc["Value"] = 3;

                response -> setLength();
                request -> send(response);
            })
        .addMiddleware( & getAlpParams);

    #pragma endregion

    #pragma region AlpacaDevice

    alpaca.on("/api/v1/switch/0/maxswitch", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        doc["Value"] = Switches.getConfiguredSwitch();

        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams});

    alpaca.on("/api/v1/switch/0/canasync", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        doc["Value"] = true;

        request -> send(response);
    }).addMiddlewares({&getAlpParams});

    alpaca.on("/api/v1/switch/0/canwrite", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        int retVal = Switches.isWritable(request -> getAttribute("id").toInt());
        if( 1 == retVal ){
            doc["Value"] = true;
        } else {
            doc["Value"] = false;
        }

        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams,&checkID});

    alpaca.on("/api/v1/switch/0/getswitch", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        doc["Value"] = Switches.getSwitchState(request -> getAttribute("id").toInt()) ? true : false;

        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams,&checkID});

    alpaca.on("/api/v1/switch/0/getswitchname", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();
        int id = request -> getAttribute("id").toInt();

        doc["Value"] = Switches.getSwitchName(id);

        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams,&checkID});

    alpaca.on("/api/v1/switch/0/getswitchdescription", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();
        int id = request -> getAttribute("id").toInt();

        doc["Value"] = Switches.getSwitchDescription(id);

        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams,&checkID});

    alpaca.on("/api/v1/switch/0/getswitchvalue", HTTP_GET,[](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();
        int id = request -> getAttribute("id").toInt();

        doc["Value"] = Switches.getSwitchState(id);

        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams,&checkID});

    alpaca.on("/api/v1/switch/0/minswitchvalue", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();
        int id = request -> getAttribute("id").toInt();

        doc["Value"] = Switches.getMin(id);

        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams,&checkID});

    alpaca.on("/api/v1/switch/0/maxswitchvalue", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();
        int id = request -> getAttribute("id").toInt();

        doc["Value"] = Switches.getMax(id);

        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams,&checkID});

    alpaca.on("/api/v1/switch/0/switchstep", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        doc["Value"] = 1;

        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams,&checkID});

    alpaca.on("/api/v1/switch/0/setswitchvalue", HTTP_PUT, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        int id = request -> getAttribute("id").toInt();
        int value = request -> getAttribute("value").toInt();

        Switches.setSwitchValue(id, value);

        response->setLength();
        request->send(response);
    }).addMiddlewares({&getAlpParams,&checkIdAndValue});

    alpaca.on("/api/v1/switch/0/setswitch", HTTP_PUT, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        int id = request -> getAttribute("id").toInt();
        int state = request -> getAttribute("state").toInt();

        int value = 0;
        if (state) {
            value = Switches.getMax(id);
        }

        Switches.setSwitchValue(id, value);

        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams,&checkIdAndState});

/* async zone */
    alpaca.on("/api/v1/switch/0/setasync", HTTP_PUT, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            int state = request->getAttribute("state").toInt();
            int id = request->getAttribute("id").toInt();
            int value = 0;
            if (state) {
              value = Switches.getMax(id);
            }
            if (Switches.getType(id) == Switches.Servo) {
                Switches.setServoPositionAsync(id,value);
            } else {
              Switches.setSwitchValue(id,value);
            }

            response->setLength();
            request->send(response);
    }).addMiddlewares({&getAlpParams, &checkIdAndState});

    alpaca.on("/api/v1/switch/0/setasyncvalue", HTTP_PUT, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            int value = request->getAttribute("value").toInt();
            int id = request->getAttribute("id").toInt();
            
            if (Switches.getType(id) == Switches.Servo) {
                Switches.setServoPositionAsync(id,value);
            } else {
              Switches.setSwitchValue(id,value);
            }

            response->setLength();
            request->send(response);
    }).addMiddlewares({&getAlpParams, &checkIdAndState});

    alpaca.on("/api/v1/switch/0/statechangecomplete", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();
        int id = request -> getAttribute("id").toInt();

        if(Switches.getType(id) != Switches.Servo){
            doc["Value"] = true;
        } else if (Switches.getType(id) != Switches.Servo){
            if(Switches.getServoIsMoving(id) == 1){
                doc["Value"] = false;
            } else {
                doc["Value"] = true;
            }
        }
        
        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams,&checkID});

    #pragma region connection

    alpaca.on("/api/v1/switch/0/connect", HTTP_PUT, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);

            response->setLength();
            request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/switch/0/disconnect", HTTP_PUT, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);

            response->setLength();
            request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/switch/0/connecting", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();

        doc["Value"] = false;

        request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/switch/0/connected", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();

        doc["Value"] = true;

        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/switch/0/connected", HTTP_PUT, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = prepareAlpacaResponse(request);

        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);

    #pragma endregion

    alpaca.on("/api/v1/switch/0/devicestate", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        JsonArray Value = doc["Value"].to<JsonArray>();
        char message[25];

        for (int i = 0; i < Switches.getConfiguredSwitch() ; i++) {
            // GetSwitch - stato bool
            JsonObject getSwitch = Value.add<JsonObject>();
            sprintf(message, "GetSwitch%d", i);
            getSwitch["Name"] = message;
            getSwitch["Value"] = Switches.getSwitchState(i) ? true : false;
            
            // GetSwitchValue - valore numerico
            JsonObject getSwitchValue = Value.add<JsonObject>();
            sprintf(message, "GetSwitchValue%d", i);
            getSwitchValue["Name"] = message;
            getSwitchValue["Value"] = Switches.getSwitchState(i);
            
            // StateChangeComplete - operazione completata
            JsonObject StateChangeComplete = Value.add<JsonObject>();
            sprintf(message, "StateChangeComplete%d", i);
            StateChangeComplete["Name"] = message;
            StateChangeComplete["Value"] = true;
        }

        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);

    /* Methods not implemented:*/
    alpaca.on("/api/v1/switch/0/commandblind", HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/switch/0/commandbool", HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/switch/0/commandstring", HTTP_PUT,alpacaMethodNotImplemented).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/switch/0/setswitchname", HTTP_PUT,alpacaMethodNotImplemented).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/switch/0/supportedactions", HTTP_GET, alpacaNoActions).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/switch/0/action", HTTP_PUT, alpacaActionNotImplemented).addMiddleware(&getAlpParams);
    #pragma endregion
}
#endif