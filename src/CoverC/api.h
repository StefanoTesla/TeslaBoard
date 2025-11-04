#ifndef COVERC_API
#define COVERC_API

extern CoverCalibratorModule CoverCalibrator;


#pragma region Middleware 


AsyncMiddlewareFunction isCoverCEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(CoverCalibrator.isEnable()){
      next();
    } else {
      request->send(500, "text/plain", "Module not enabled");
    }
    
});
AsyncMiddlewareFunction isCoverEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(CoverCalibrator.cover.isEnable()){
      next();
    } else {
      request->send(500, "text/plain", "Module not enabled");
    }
    
});
AsyncMiddlewareFunction isCalibratorEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(CoverCalibrator.calibrator.isEnable()){
      next();
    } else {
      request->send(500, "text/plain", "Module not enabled");
    }
    
});

AsyncMiddlewareFunction checkBrightness([](AsyncWebServerRequest *request, ArMiddlewareNext next) {
    	String id = request->getAttribute("brightness", String("null"));
		if (id == "null") {
			missingIdErrorMessage(request);
			return;
		} else {
			next();
		}
});

#pragma endregion 

void CoverCalibratorApi(){

#pragma region webAPI

    server.on("/api/coverc/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        CoverCalibrator.getConfiguration(doc);

        response->setLength();
        request->send(response);
    });

    server.on("/api/coverc/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonObject calibrator = doc["calibrator"].to<JsonObject>();
        calibrator["status"] = CoverCalibrator.calibrator.getStatus();
        if(CoverCalibrator.calibrator.isEnable()){
            calibrator["brightness"] = CoverCalibrator.calibrator.getBrightness();
        }

        JsonObject cover = doc["cover"].to<JsonObject>();
        cover["status"] = CoverCalibrator.cover.getStatus();
        if(CoverCalibrator.cover.isEnable()){
            cover["angle"] = CoverCalibrator.cover.getPosition(); /* TODO */
        }
        
        response->setLength();
        request->send(response);
    }).addMiddleware(&isCoverCEnable); 

    server.on("/api/coverc/open", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        
        if(CoverCalibrator.cover.canOpen()){
            doc["execute"] = true;
            CoverCalibrator.cover.open();
        } else {
            if(CoverCalibrator.cover.isMoving()){
                doc["error"] = "coverIsMoving";
            } else if(CoverCalibrator.cover.isOpen()){
                doc["error"] = "coverIsOpen";
            }
        }

        response->setLength();
        request->send(response);
    }).addMiddleware(&isCalibratorEnable);
    
    server.on("/api/coverc/halt", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        CoverCalibrator.cover.halt();
        doc["execute"] = true;

        response->setLength();
        request->send(response);
    }).addMiddleware(&isCoverEnable);

    server.on("/api/coverc/close", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        if(CoverCalibrator.cover.canClose()){
            doc["execute"] = true;
            CoverCalibrator.cover.close();
        } else {
            if(CoverCalibrator.cover.isMoving()){
                doc["error"] = "coverIsMoving";
            } else if(CoverCalibrator.cover.isClosed()){
                doc["error"] = "coverIsOpen";
            }
        }

        response->setLength();
        request->send(response);
    }).addMiddleware(&isCoverEnable);

    server.on("/api/coverc/brightness", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        String parameter;
        bool present = false;
        bool inRange = false;
        int value = 0;

        doc["execute"] = false;

        int paramsNr = request->params();

        for (int i = 0; i < paramsNr; i++) {
            const AsyncWebParameter* p = request->getParam(i);
            parameter = p->name();
            if (parameter == "brightness") {
                present = true;
                value = p->value().toInt();
                if(value >=0 && value <= CoverCalibrator.calibrator.getMaxBrightness()){
                    CoverCalibrator.calibrator.setBrightness(value);
                    inRange = true;
                    doc["execute"] = true;
                }
                else{
                    inRange = false;
                }
            }  
        }
        if(!present){
            doc["error"] = "calibBrightnessNotPresent";
        } else {
            if(!inRange){
                doc["error"] = "calibBrightnessNotInRange";
            }
        }

        response->setLength();
        request->send(response);
    }).addMiddleware(&isCalibratorEnable); 

    server.on("/api/coverc/on", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        doc["execute"] = false;

        CoverCalibrator.calibrator.setBrightness(4095);
        doc["execute"] = true;

        response->setLength();
        request->send(response);
    }).addMiddlewares({&isCoverCEnable,&isCalibratorEnable}); 

    server.on("/api/coverc/off", HTTP_POST, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        CoverCalibrator.calibrator.setBrightness(0);

        doc["execute"] = true;

        response->setLength();
        request->send(response);
    }).addMiddleware(&isCalibratorEnable); 


    AsyncCallbackJsonWebHandler* coverCConfigHandler = new AsyncCallbackJsonWebHandler("/api/coverc/cfg");

    coverCConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
    coverCConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& root) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        
        //convert the JsonVariant to JsonObject
        const JsonObject& incomingObj = root.as<JsonObject>();

        JsonArray err = doc["errors"].to<JsonArray>();

        CoverCalibrator.validateConfiguration(incomingObj,doc);

        if(err.size()>0){
            response->setCode(500);
            response->setLength();
            request->send(response);
            return;
        }


        CoverCalibrator.storeConfiguration(incomingObj);
        
        response->setLength();
        request->send(response);
        
    });

    server.addHandler(coverCConfigHandler);

#pragma endregion

#pragma region AlpacaDevice


  alpaca.on("/api/v1/covercalibrator/0/brightness", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();

        if(!CoverCalibrator.calibrator.isEnable()){
            alpacaPropertyNotImplemented(request);
            return;
        }

        doc["Value"] = CoverCalibrator.calibrator.getBrightness();
        response->setLength();
        request->send(response);
  }).addMiddleware(&getAlpParams);

  alpaca.on("/api/v1/covercalibrator/0/calibratorchanging", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();
      
      doc["Value"] = false;
      
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpParams);

  alpaca.on("/api/v1/covercalibrator/0/calibratorstate", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = CoverCalibrator.calibrator.getStatus();
        response->setLength();
        request->send(response);
  }).addMiddleware(&getAlpParams);


alpaca.on("/api/v1/covercalibrator/0/maxbrightness", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = CoverCalibrator.calibrator.getMaxBrightness();
        response->setLength();
        request->send(response);
  }).addMiddleware(&getAlpParams);


    alpaca.on("/api/v1/covercalibrator/0/calibratoron", HTTP_PUT, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();

        CoverCalibrator.calibrator.setBrightness(request->getAttribute("brightness").toInt());

        response->setLength();
        request->send(response);
    }).addMiddlewares({&getAlpParams,&checkBrightness});


    alpaca.on("/api/v1/covercalibrator/0/calibratoroff", HTTP_PUT, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();

        CoverCalibrator.calibrator.setBrightness(0);
      
        response->setLength();
        request->send(response); 
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/covercalibrator/0/covermoving", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = CoverCalibrator.cover.isMoving();
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/covercalibrator/0/coverstate", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = CoverCalibrator.cover.getStatus();
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);



    alpaca.on("/api/v1/covercalibrator/0/devicestate", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        JsonArray Value = doc["Value"].to<JsonArray>();
        JsonObject calibStatus = Value.add<JsonObject>();
        calibStatus["Name"] = "CalibratorState";
        calibStatus["Value"] = CoverCalibrator.calibrator.getStatus();
        JsonObject calibChanging = Value.add<JsonObject>();
        calibChanging["Name"] = "CalibratorChanging";
        calibChanging["Value"] = false;
        JsonObject coverState = Value.add<JsonObject>();
        coverState["Name"] = "CoverState";
        coverState["Value"] = CoverCalibrator.cover.getStatus();
        JsonObject coverMoving = Value.add<JsonObject>();
        coverMoving["Name"] = "CoverMoving";
        coverMoving["Value"] = CoverCalibrator.cover.isMoving();
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);



  alpaca.on("/api/v1/covercalibrator/0/closecover", HTTP_PUT, [](AsyncWebServerRequest *request){
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();

      if(!CoverCalibrator.cover.isEnable()){
            alpacaMethodNotImplemented(request);
            return;
      }
      
      if(CoverCalibrator.cover.isMoving()){
            doc["ErrorNumber"] = 1035;
            doc["ErrorMessage"] = "Cover is moving";
            response->setLength();
            request->send(response);
            return;
      }
      
      CoverCalibrator.cover.close();
      
      response->setLength();
      request->send(response); 
  }).addMiddleware(&getAlpParams);

  alpaca.on("/api/v1/covercalibrator/0/opencover", HTTP_PUT, [](AsyncWebServerRequest *request){
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();

      if(!CoverCalibrator.cover.isEnable()){
            alpacaMethodNotImplemented(request);
            return;
      }
      
      if(CoverCalibrator.cover.isMoving()){
            doc["ErrorNumber"] = 1035;
            doc["ErrorMessage"] = "Cover is moving";
            response->setLength();
            request->send(response);
            return;
      }

      CoverCalibrator.cover.open();
      
      response->setLength();
      request->send(response); 
  }).addMiddleware(&getAlpParams);


    /* Property not implemented:*/
    alpaca.on("/api/v1/covercalibrator/0/haltcover",        HTTP_PUT, alpacaPropertyNotImplemented).addMiddleware(&getAlpParams);


    /* Methods not implemented:*/
    alpaca.on("/api/v1/covercalibrator/0/commandblind",     HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/covercalibrator/0/commandbool",      HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/covercalibrator/0/commandstring",    HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpParams);

   /* I don't care about connection but we need to declare it*/
    alpaca.on("/api/v1/covercalibrator/0/connect", HTTP_PUT, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        response->setLength();
        request->send(response);

    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/covercalibrator/0/disconnect", HTTP_PUT, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        response->setLength();
        request->send(response);

    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/covercalibrator/0/connecting", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = false;
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/covercalibrator/0/connected", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["Value"] = true;
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/covercalibrator/0/connected", HTTP_PUT, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/covercalibrator/0/supportedactions",HTTP_GET, alpacaNoActions).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/covercalibrator/0/action",HTTP_PUT, alpacaActionNotImplemented).addMiddleware(&getAlpParams);


#pragma endregion

#pragma region AlpacaManage


  alpaca.on("/api/v1/covercalibrator/0/name", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();
      doc["Value"] = CoverCalibrator.getIdentifier() + " - TeslaBoard";
      response->setLength();
      request->send(response);
  }).addMiddlewares({&isCalibratorEnable,&getAlpParams});

  alpaca.on("/api/v1/covercalibrator/0/description",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();
      doc["Value"] = "CoverCalibrator by Stefano TeslaBoard";
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpParams);

  alpaca.on("/api/v1/covercalibrator/0/driverversion",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    doc["Value"] = SW_VERSION;
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpParams);

  alpaca.on("/api/v1/covercalibrator/0/driverinfo",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = "New version with 20kHz flat panel and cover support";
    
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpParams);

  alpaca.on("/api/v1/covercalibrator/0/interfaceversion",                                         HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = 2;

    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpParams);

#pragma endregion


}


#endif