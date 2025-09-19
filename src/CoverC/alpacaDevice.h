#ifndef CC_ALPACA_DEVICE
#define CC_ALPACA_DEVICE


AsyncMiddlewareFunction getBrightness([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
  int paramsNr = request->params();
  String parameter;

  request->setAttribute("brightness",  static_cast<long>(-1));
  for (int i = 0; i < paramsNr; i++) {
    const AsyncWebParameter* p = request->getParam(i);
    parameter = p->name();
    parameter.toLowerCase();
    if (parameter == "brightness") {
      request->setAttribute("brightness",  p->value().toInt());
      next();
    }
  }
  
  next();
});


void coverAlpacaDevice(){

  alpaca.on("/api/v1/covercalibrator/0/brightness", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();

        if(!CoverC.config.calibrator.present){
            alpacaPropertyNotImplemented(request);
            return;
        }

        doc["Value"] = Calibrator.status();
        response->setLength();
        request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/covercalibrator/0/calibratorchanging", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();
      
      doc["Value"] = false;
      
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/covercalibrator/0/calibratorstate", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = CoverC.status.calibrator.status;
        response->setLength();
        request->send(response);
  }).addMiddleware(&getAlpacaID);


alpaca.on("/api/v1/covercalibrator/0/maxbrightness", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = Calibrator.getMax();
        response->setLength();
        request->send(response);
  }).addMiddleware(&getAlpacaID);


    alpaca.on("/api/v1/covercalibrator/0/calibratoron", HTTP_PUT, [](AsyncWebServerRequest *request){
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();

      if(!CoverC.config.calibrator.present){
             alpacaMethodNotImplemented(request);
             return;
      }
      int br = request->getAttribute("brightness").toInt();
      if(br == -1){
            doc["ErrorNumber"] = 1025;
            doc["ErrorMessage"] = "Brightness parameter not found";
      } else if(br < 0 or br > Calibrator.getMax()){
            doc["ErrorNumber"] = 1025;
            doc["ErrorMessage"] = "Brightness outside range";
      } else {
            Calibrator.write(br);
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
      }
      response->setLength();
      request->send(response);
  }).addMiddlewares({&getAlpacaID,&getBrightness});


  alpaca.on("/api/v1/covercalibrator/0/calibratoroff", HTTP_PUT, [](AsyncWebServerRequest *request){
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();

      if(!CoverC.config.calibrator.present){
             alpacaMethodNotImplemented(request);
             return;
      }
      
      Calibrator.write(0);
      
      response->setLength();
      request->send(response); 
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/covercalibrator/0/covermoving", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = CoverC.status.cover.status == CoverStatusMoving ? true : false;
        response->setLength();
        request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/covercalibrator/0/coverstate", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = CoverC.status.cover.status;
        response->setLength();
        request->send(response);
  }).addMiddleware(&getAlpacaID);



  alpaca.on("/api/v1/covercalibrator/0/devicestate", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    JsonArray Value = doc["Value"].to<JsonArray>();
    JsonObject calibStatus = Value.add<JsonObject>();
    calibStatus["Name"] = "CalibratorState";
    calibStatus["Value"] = CoverC.status.calibrator.status;
    JsonObject calibChanging = Value.add<JsonObject>();
    calibChanging["Name"] = "CalibratorChanging";
    calibChanging["Value"] = false;
    JsonObject coverState = Value.add<JsonObject>();
    coverState["Name"] = "CoverState";
    coverState["Value"] = CoverC.status.cover.status;
    JsonObject coverMoving = Value.add<JsonObject>();
    coverMoving["Name"] = "CoverMoving";
    coverMoving["Value"] = false;
    response->setLength();
    request->send(response);
}).addMiddleware(&getAlpacaID);



  alpaca.on("/api/v1/covercalibrator/0/closecover", HTTP_PUT, [](AsyncWebServerRequest *request){
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();

      if(!CoverC.config.cover.present){
            alpacaMethodNotImplemented(request);
            return;
      }
      
      if(CoverC.status.cover.status == 2){
            doc["ErrorNumber"] = 1035;
            doc["ErrorMessage"] = "Cover is moving";
            response->setLength();
            request->send(response);
            return;
      }
      
      CoverC.command.cover.move = true;
      CoverC.command.cover.angle = Cover.closeDeg;
      
      response->setLength();
      request->send(response); 
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/covercalibrator/0/opencover", HTTP_PUT, [](AsyncWebServerRequest *request){
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();

      if(!CoverC.config.cover.present){
            alpacaMethodNotImplemented(request);
            return;
      }
      
      if(CoverC.status.cover.status == 2){
            doc["ErrorNumber"] = 1035;
            doc["ErrorMessage"] = "Cover is moving";
            response->setLength();
            request->send(response);
            return;
      }

      CoverC.command.cover.move = true;
      CoverC.command.cover.angle = Cover.openDeg;
      
      response->setLength();
      request->send(response); 
  }).addMiddleware(&getAlpacaID);


    /* Property not implemented:*/
    alpaca.on("/api/v1/covercalibrator/0/haltcover",        HTTP_PUT, alpacaPropertyNotImplemented).addMiddleware(&getAlpacaID);


    /* Methods not implemented:*/
    alpaca.on("/api/v1/covercalibrator/0/commandblind",     HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpacaID);
    alpaca.on("/api/v1/covercalibrator/0/commandbool",      HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpacaID);
    alpaca.on("/api/v1/covercalibrator/0/commandstring",    HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpacaID);

   /* I don't care about connection but we need to declare it*/
    alpaca.on("/api/v1/covercalibrator/0/connect", HTTP_PUT, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        response->setLength();
        request->send(response);

    }).addMiddleware(&getAlpacaID);

    alpaca.on("/api/v1/covercalibrator/0/disconnect", HTTP_PUT, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        response->setLength();
        request->send(response);

    }).addMiddleware(&getAlpacaID);

    alpaca.on("/api/v1/covercalibrator/0/connecting", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = false;
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpacaID);

    alpaca.on("/api/v1/covercalibrator/0/connected", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = true;
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpacaID);

    alpaca.on("/api/v1/covercalibrator/0/connected", HTTP_PUT, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpacaID);

    alpaca.on("/api/v1/covercalibrator/0/supportedactions",HTTP_GET, alpacaNoActions).addMiddleware(&getAlpacaID);
    alpaca.on("/api/v1/covercalibrator/0/action",HTTP_PUT, alpacaActionNotImplemented).addMiddleware(&getAlpacaID);
}

#endif