#ifndef DOME_ALPACA_DEVICE
#define DOME_ALPACA_DEVICE

AsyncMiddlewareFunction upLastCom([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
  Dome.Shutter.lastCommunicationMillis  = millis();
  next();
});

void domeAlpacaDevices(){ 

alpaca.on("/api/v1/dome/0/shutterstatus",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = Dome.Shutter.status;

    response->setLength();
    request->send(response);

}).addMiddlewares({&getAlpacaID,&upLastCom});


alpaca.on("/api/v1/dome/0/closeshutter",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    if(Dome.Shutter.status != ShStatusClose and Dome.Shutter.command == ShCommandIdle){
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      Dome.Shutter.command = ShCommandClose;
    } else {
      doc["ErrorNumber"] = 1035;
      if(Dome.Shutter.status == ShStatusClose){
        doc["ErrorMessage"] = "Shutter was already closed";
      } else {
        doc["ErrorMessage"] = "Shutter is busy, another command in progress";
      }
    }

    response->setLength();
    request->send(response);

}).addMiddlewares({&getAlpacaID,&upLastCom});

alpaca.on("/api/v1/dome/0/openshutter",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    if(Dome.Shutter.status != ShStatusOpen and Dome.Shutter.command == ShCommandIdle){
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      Dome.Shutter.command = ShCommandOpen;
    } else {
      doc["ErrorNumber"] = 1035;
      if(Dome.Shutter.status == ShStatusOpen){
        doc["ErrorMessage"] = "Shutter was already apened";
      } else {
        doc["ErrorMessage"] = "Shutter is busy, another command in progress";
      }
    }

    response->setLength();
    request->send(response);

}).addMiddlewares({&getAlpacaID,&upLastCom});


alpaca.on("/api/v1/dome/0/abortslew",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    Dome.Shutter.command = ShCommandHalt;
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    response->setLength();
    request->send(response);

}).addMiddlewares({&getAlpacaID,&upLastCom});

alpaca.on("/api/v1/dome/0/cansetshutter",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = true;

    response->setLength();
    request->send(response);
}).addMiddlewares({&getAlpacaID,&upLastCom});

alpaca.on("/api/v1/dome/0/slewing",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = Dome.Shutter.command == ShCommandIdle ? false : true;

    response->setLength();
    request->send(response);
}).addMiddlewares({&getAlpacaID,&upLastCom});

alpaca.on("/api/v1/dome/0/devicestate",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    JsonArray Value = doc["Value"].to<JsonArray>();
    JsonObject shstatus = Value.add<JsonObject>();
    shstatus["Name"] = "ShutterStatus";
    shstatus["Value"] = Dome.Shutter.status;
    JsonObject shslewing = Value.add<JsonObject>();
    shslewing["Name"] = "Slewing";
    shslewing["Value"] = Dome.Shutter.command == ShCommandIdle ? false : true;

    response->setLength();
    request->send(response);
}).addMiddlewares({&getAlpacaID,&upLastCom});

/* I don't care about connection but we need to declare it*/
alpaca.on("/api/v1/dome/0/connect",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);

    response->setLength();
    request->send(response);

}).addMiddlewares({&getAlpacaID,&upLastCom});

alpaca.on("/api/v1/dome/0/disconnect",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);

    response->setLength();
    request->send(response);

}).addMiddlewares({&getAlpacaID,&upLastCom});

alpaca.on("/api/v1/dome/0/connecting",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = false;

    response->setLength();
    request->send(response);
}).addMiddlewares({&getAlpacaID,&upLastCom});

alpaca.on("/api/v1/dome/0/connected",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = true;

    response->setLength();
    request->send(response);
}).addMiddlewares({&getAlpacaID,&upLastCom});

alpaca.on("/api/v1/dome/0/connected",                                            HTTP_PUT, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);

    response->setLength();
    request->send(response);
}).addMiddlewares({&getAlpacaID,&upLastCom});


/* things this board can't do:*/
alpaca.on("/api/v1/dome/0/canfindhome",     HTTP_GET,alpacaCant).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/canpark",         HTTP_GET,alpacaCant).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/cansetaltitude",  HTTP_GET,alpacaCant).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/cansetazimuth",   HTTP_GET,alpacaCant).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/cansetpark",      HTTP_GET,alpacaCant).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/canslave",        HTTP_GET,alpacaCant).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/cansyncazimuth",  HTTP_GET,alpacaCant).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/slaved",          HTTP_GET,alpacaCant).addMiddlewares({&getAlpacaID,&upLastCom});

/* Property not implemented:*/
alpaca.on("/api/v1/dome/0/altitude",        HTTP_GET, alpacaPropertyNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/athome",          HTTP_GET, alpacaPropertyNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/atpark",          HTTP_GET, alpacaPropertyNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/azimuth",         HTTP_GET, alpacaPropertyNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});



/* Methods not implemented:*/
alpaca.on("/api/v1/dome/0/slaved",          HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/findhome",        HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/park",            HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/setpark",         HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/slewtoaltitude",  HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/slewtoazimuth",   HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/synctoazimuth",   HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/commandblind",    HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/commandbool",     HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/commandstring",   HTTP_PUT, alpacaMethodNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});

alpaca.on("/api/v1/dome/0/supportedactions",HTTP_GET, alpacaNoActions).addMiddlewares({&getAlpacaID,&upLastCom});
alpaca.on("/api/v1/dome/0/action",HTTP_PUT, alpacaActionNotImplemented).addMiddlewares({&getAlpacaID,&upLastCom});

}

#endif