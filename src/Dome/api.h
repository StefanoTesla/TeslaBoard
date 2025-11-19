#ifndef DOME_API
#define DOME_API
extern DomeModule Dome;


#pragma region Middleware 
AsyncMiddlewareFunction isDomeEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(Dome.isEnable()){
      next();
    } else {
      request->send(500, "application/json", "{\"enable\":false}");
    }
    
});


AsyncMiddlewareFunction upLastCom([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
  Dome.shutter.updateLastCommunication();
  next();
});

#pragma endregion

void DomeApi(){

#pragma region webApi

    server.on("/api/dome/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        Dome.getConfiguration(doc);
        
        response->setLength();
        request->send(response);
    });

    server.on("/api/dome/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["enable"] = Dome.isEnable();

        JsonObject shutter = doc["shutter"].to<JsonObject>();
            shutter["roofState"] = Dome.shutter.getStatus();
            shutter["actualCommand"] = Dome.shutter.getActualCommand();
            shutter["canOpen"] = Dome.shutter.canOpen();
            shutter["canClose"] = Dome.shutter.canClose();
            shutter["lastTravelTime"] = Dome.shutter.lastTravelTime();

        JsonObject input = shutter["input"].to<JsonObject>();
            input["open"] = Dome.shutter.getOpenSensorRaw();
            input["close"] = Dome.shutter.getCloseSensorRaw();

        JsonObject autoclose = shutter["autoClose"].to<JsonObject>();
            autoclose["enable"] = Dome.shutter.isAutoCloseEnable();

        response->setLength();
        request->send(response);
    }).addMiddleware(&isDomeEnable);

    server.on("/api/dome/open", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = false;

        if(Dome.shutter.isMoving())
        {
            doc["error"] = "ShAlreadyMoving";
            response->setLength();
            request->send(response);
            return;
        }

        if (Dome.shutter.isOpen()){
            doc["error"] = "ShAlreadyOpen";
            response->setLength();
            request->send(response);
            return;
        }
        
        doc["execute"] = true;
        Dome.shutter.open();
        
        response->setLength();
        request->send(response);
    }).addMiddleware(&isDomeEnable);

    server.on("/api/dome/close", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = false;

        if(Dome.shutter.isMoving())
        {
            doc["error"] = "ShAlreadyMoving";
            response->setLength();
            request->send(response);
            return;
        }

        if (Dome.shutter.isClosed()){
            doc["error"] = "ShAlreadyOpen";
            response->setLength();
            request->send(response);
            return;
        }

        doc["execute"] = true;
        Dome.shutter.close();

        response->setLength();
        request->send(response);
    }).addMiddleware(&isDomeEnable);

    server.on("/api/dome/halt", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = true;

        Dome.shutter.halt();
        response->setLength();
        request->send(response);
    }).addMiddleware(&isDomeEnable);

    AsyncCallbackJsonWebHandler* domeConfigHandler = new AsyncCallbackJsonWebHandler("/api/dome/cfg");

    domeConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
    domeConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& root) {
        
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        
        //convert the JsonVariant to JsonObject
        const JsonObject& incomingObj = root.as<JsonObject>();

        JsonArray err = doc["errors"].to<JsonArray>();

        Dome.validateConfiguration(incomingObj,doc);

        if(err.size()>0){
            response->setCode(500);
            response->setLength();
            request->send(response);
            return;
        }


        Dome.storeConfiguration(incomingObj);
        
        response->setLength();
        request->send(response);
    });

    server.addHandler(domeConfigHandler);

#pragma endregion

#pragma region aplacaManage


  alpaca.on("/api/v1/dome/0/name",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot().to<JsonObject>();

      doc["Value"] = Dome.getIdentifier() + " - TeslaBoard";

      response->setLength();
      request->send(response);
  }).addMiddlewares({&isDomeEnable,&getAlpParams});

  alpaca.on("/api/v1/dome/0/description",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot().to<JsonObject>();

      doc["Value"] = "Dome handler R.O.R. by Stefano TeslaBoard";

      response->setLength();
      request->send(response);
  }).addMiddlewares({&isDomeEnable,&getAlpParams});

  alpaca.on("/api/v1/dome/0/driverversion",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot().to<JsonObject>();

    doc["Value"] = SW_VERSION;
    
    response->setLength();
    request->send(response);
  }).addMiddlewares({&isDomeEnable,&getAlpParams});

  alpaca.on("/api/v1/dome/0/driverinfo",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot().to<JsonObject>();

      doc["Value"] = "Using the IOManager and three way to handle r.o.r.";

      response->setLength();
      request->send(response);
  }).addMiddlewares({&isDomeEnable,&getAlpParams});

  alpaca.on("/api/v1/dome/0/interfaceversion",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot().to<JsonObject>();

    doc["Value"] = 3;

    response->setLength();
    request->send(response);
  }).addMiddlewares({&isDomeEnable,&getAlpParams});



#pragma endregion

#pragma region alpacaDevice


alpaca.on("/api/v1/dome/0/shutterstatus",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = Dome.shutter.getStatus();

    response->setLength();
    request->send(response);

}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});


alpaca.on("/api/v1/dome/0/closeshutter",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    if(Dome.shutter.canClose()){
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      Dome.shutter.close();
    } else {
      doc["ErrorNumber"] = 1035;
      if(Dome.shutter.isClosed()){
        doc["ErrorMessage"] = "Shutter was already closed";
      } else {
        doc["ErrorMessage"] = "Shutter is busy, another command in progress";
      }
    }

    response->setLength();
    request->send(response);

}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});

alpaca.on("/api/v1/dome/0/openshutter",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    if(Dome.shutter.canOpen()){
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      Dome.shutter.open();
    } else {
      doc["ErrorNumber"] = 1035;
      if(Dome.shutter.isOpen()){
        doc["ErrorMessage"] = "Shutter was already apened";
      } else {
        doc["ErrorMessage"] = "Shutter is busy, another command in progress";
      }
    }

    response->setLength();
    request->send(response);

}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});


alpaca.on("/api/v1/dome/0/abortslew",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    //Dome.Shutter.command = ShCommandHalt;
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    response->setLength();
    request->send(response);

}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});

alpaca.on("/api/v1/dome/0/cansetshutter",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = true;

    response->setLength();
    request->send(response);
}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});

alpaca.on("/api/v1/dome/0/slewing",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = Dome.shutter.isMoving() ? true : false;

    response->setLength();
    request->send(response);
}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});

alpaca.on("/api/v1/dome/0/devicestate",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    JsonArray Value = doc["Value"].to<JsonArray>();
    JsonObject shstatus = Value.add<JsonObject>();
    shstatus["Name"] = "ShutterStatus";
    shstatus["Value"] = Dome.shutter.getStatus();
    JsonObject shslewing = Value.add<JsonObject>();
    shslewing["Name"] = "Slewing";
    shslewing["Value"] = false; //Dome.Shutter.command == ShCommandIdle ? false : true; /* TODO */

    response->setLength();
    request->send(response);
}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});

/* I don't care about connection but we need to declare it*/
alpaca.on("/api/v1/dome/0/connect",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);

    response->setLength();
    request->send(response);

}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});

alpaca.on("/api/v1/dome/0/disconnect",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);

    response->setLength();
    request->send(response);

}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});

alpaca.on("/api/v1/dome/0/connecting",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = false;

    response->setLength();
    request->send(response);
}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});

alpaca.on("/api/v1/dome/0/connected",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = true;

    response->setLength();
    request->send(response);
}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});

alpaca.on("/api/v1/dome/0/connected",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);

    response->setLength();
    request->send(response);
}).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});


/* things this board can't do:*/
alpaca.on("/api/v1/dome/0/canfindhome",     HTTP_GET,alpacaCant).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/canpark",         HTTP_GET,alpacaCant).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/cansetaltitude",  HTTP_GET,alpacaCant).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/cansetazimuth",   HTTP_GET,alpacaCant).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/cansetpark",      HTTP_GET,alpacaCant).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/canslave",        HTTP_GET,alpacaCant).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/cansyncazimuth",  HTTP_GET,alpacaCant).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/slaved",          HTTP_GET,alpacaCant).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});

/* Property not implemented:*/
alpaca.on("/api/v1/dome/0/altitude",        HTTP_GET, alpacaPropertyNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/athome",          HTTP_GET, alpacaPropertyNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/atpark",          HTTP_GET, alpacaPropertyNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/azimuth",         HTTP_GET, alpacaPropertyNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});



/* Methods not implemented:*/
alpaca.on("/api/v1/dome/0/slaved",          HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/findhome",        HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/park",            HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/setpark",         HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/slewtoaltitude",  HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/slewtoazimuth",   HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/synctoazimuth",   HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/commandblind",    HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/commandbool",     HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/commandstring",   HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});

alpaca.on("/api/v1/dome/0/supportedactions",HTTP_GET, alpacaNoActions).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});
alpaca.on("/api/v1/dome/0/action",HTTP_PUT, alpacaActionNotImplemented).addMiddlewares({&isDomeEnable,&getAlpParams,&upLastCom});


#pragma endregion

}
#endif