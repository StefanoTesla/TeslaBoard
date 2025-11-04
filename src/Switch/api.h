#ifndef SWITCH_WEBSERVER
#define SWITCH_WEBSERVER

extern SwitchModule Switches;

#pragma region ErrorsHandler

void unWritableIdErrorMessage(AsyncWebServerRequest *request) {
  AsyncJsonResponse *response = prepareAlpacaResponse(request);
  JsonObject doc = response->getRoot().to<JsonObject>();
  char message[100];
  int id = request->getAttribute("id").toInt();
  sprintf(message, "Switch n: %d, cannot be written", id);
  doc["ErrorNumber"] = 1025;
  doc["ErrorMessage"] = message;
  response->setLength();
  response->setCode(400);
  request->send(response);
}

void missingIdErrorMessage(AsyncWebServerRequest *request) {
  AsyncJsonResponse *response = prepareAlpacaResponse(request);
  JsonObject doc = response->getRoot();

  doc["ErrorNumber"] = 1025;
  doc["ErrorMessage"] = "ID not provided";

  response->setLength();
  response->setCode(400);
  request->send(response);
}

void IdOutOfRangeErrorMessage(AsyncWebServerRequest *request) {
  AsyncJsonResponse *response = prepareAlpacaResponse(request);
  JsonObject doc = response->getRoot();
  char message[100];
  int maxID = Switches.getConfiguredSwitch();
  int id = request->getAttribute("id").toInt();
  sprintf(message, "ID provided: %d, outside range, maximum is: %d - 1", id,
          maxID);
  doc["ErrorNumber"] = 1025;
  doc["ErrorMessage"] = message;

  response->setLength();
  response->setCode(400);
  request->send(response);
}

void missingStateErrorMessage(AsyncWebServerRequest *request) {
  AsyncJsonResponse *response = prepareAlpacaResponse(request);
  JsonObject doc = response->getRoot().to<JsonObject>();
  char message[100];
  sprintf(message, "\"State\" parameter not provided");
  doc["ErrorNumber"] = 1025;
  doc["ErrorMessage"] = message;
  response->setLength();
  response->setCode(400);
  request->send(response);
}

void missingValueErrorMessage(AsyncWebServerRequest *request) {
  AsyncJsonResponse *response = prepareAlpacaResponse(request);
  JsonObject doc = response->getRoot();

  doc["ErrorNumber"] = 1025;
  doc["ErrorMessage"] = "\"Value\" parameter not provided";

  response->setLength();
  response->setCode(400);
  request->send(response);
}

void valueOutOfRangeErrorMessage(AsyncWebServerRequest *request) {
  AsyncJsonResponse *response = prepareAlpacaResponse(request);
  JsonObject doc = response->getRoot();
  char message[100];
  int id = request->getAttribute("id").toInt();
  int value = request->getAttribute("value").toInt();
  sprintf(message, "ID provided: %d, outside range, maximum is: %d", id, value);
  doc["ErrorNumber"] = 1025;
  doc["ErrorMessage"] = message;

  response->setLength();
  response->setCode(400);
  request->send(response);
}
#pragma endregion

#pragma region Middleware

AsyncMiddlewareFunction modEN([](AsyncWebServerRequest *request,
                                 ArMiddlewareNext next) {
  if (Switches.isEnable()) {
    next();
  } else {
    request->send(403, "text/plain", "Forbidden");
  }
});

AsyncMiddlewareFunction checkID([](AsyncWebServerRequest *request,
                                   ArMiddlewareNext next) {
  String id = request->getAttribute("id", String("null"));
  if (id == "null") {
    missingIdErrorMessage(request);
    return;
  } else {
    int i = id.toInt();

    if (i < 0 && i >= Switches.getConfiguredSwitch()) {
      IdOutOfRangeErrorMessage(request);
      return;
    } else {
      next();
    }
  }
});
AsyncMiddlewareFunction checkValue([](AsyncWebServerRequest *request,
                                      ArMiddlewareNext next) {
  String id = request->getAttribute("value", String("null"));
  if (id == "null") {
    missingIdErrorMessage(request);
    return;
  } else {
    next();
  }
});
AsyncMiddlewareFunction checkState([](AsyncWebServerRequest *request,
                                      ArMiddlewareNext next) {
  String id = request->getAttribute("state", String("null"));
  if (id == "null") {
    missingIdErrorMessage(request);
    return;
  } else {
    next();
  }
});

AsyncMiddlewareFunction isSettable([](AsyncWebServerRequest *request, ArMiddlewareNext next) {

  if (Switches.canBeWrite(request->getAttribute("id").toInt())){
    next();
  }
  unWritableIdErrorMessage(request);
  return;
    
});

#pragma endregion

void webApi() {

#pragma region webApi

  server.on("/api/switch/cfg", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    doc["enable"] = Switches.isEnable();
    doc["order"] = Switches.uiOrder;
    doc["identifier"] = Switches.getIdentifier();

    Switches.getConfiguration(doc);

    response->setLength();
    request->send(response);
  });

  server.on("/api/switch/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    JsonArray array = doc["Switches"].to<JsonArray>();

    for (size_t i = 0; i < Switches.getConfiguredSwitch(); i++) {
      JsonObject swi = array.add<JsonObject>();
      Switches.reportSwitchState(i, swi);
    }

    response->setLength();
    request->send(response);
  });

  server.on("/api/switch/set-value", HTTP_POST,
            [](AsyncWebServerRequest *request) {
              AsyncJsonResponse *response = new AsyncJsonResponse();
              JsonObject doc = response->getRoot().to<JsonObject>();
              String parameter;
              doc["execute"] = false;
              bool inRange = false;
              int id = -1;
              int value = -1;
              int paramsNr = request->params();

              for (int i = 0; i < paramsNr; i++) {
                const AsyncWebParameter *p = request->getParam(i);
                parameter = p->name();
                if (parameter == "id") {
                  id = p->value().toInt();
                }
                if (parameter == "value") {
                  value = p->value().toInt();
                }
              }

              if (id == -1) {
                doc["error"] = "SwIdOAbsent";
                response->setLength();
                request->send(response);
                return;
              }

              if (value == -1) {
                doc["error"] = "SwValueAbsent";
                response->setLength();
                request->send(response);
                return;
              }

              switch (Switches.setSwitchState(id, value)) {
              case 1:
                doc["execute"] = true;
                break;
              case -1:
                doc["error"] = "SwIdOutOfRange";
                break;
              case -2:
                doc["error"] = "SwValueBehindMin";
                break;
              case -3:
                doc["error"] = "SwValueOverMax";
                break;
              case -4:
                doc["error"] = "SwNotWritable";
                break;
              default:
                doc["error"] = "Undefined Error";
                break;
              }
              Switches.setSwitchState(id, value);

              response->setLength();
              request->send(response);
            });

  AsyncCallbackJsonWebHandler *switchConfigHandler =
      new AsyncCallbackJsonWebHandler("/api/switch/cfg");

  switchConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
  switchConfigHandler->onRequest(
      [](AsyncWebServerRequest *request, JsonVariant &root) {
        AsyncJsonResponse *response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        // convert the JsonVariant to JsonObject
        const JsonObject &incomingObj = root.as<JsonObject>();

        JsonArray err = doc["errors"].to<JsonArray>();

        Switches.validateConfiguration(incomingObj, doc);

        if (err.size() > 0) {
          response->setCode(500);
          response->setLength();
          request->send(response);
          return;
        }

        Switches.storeConfiguration();

        response->setLength();
        request->send(response);
      });

  server.addHandler(switchConfigHandler);

#pragma endregion

#pragma region AlpachaManagement

  alpaca
      .on("/api/v1/switch/0/name", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = Switches.getIdentifier() + " - TeslaBoard";

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpParams);

  alpaca
      .on("/api/v1/switch/0/description", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = "Switch handled by Stefano TeslaBoard";

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpParams);

  alpaca.on("/api/v1/switch/0/driverversion", HTTP_GET,
            [](AsyncWebServerRequest *request) {
              AsyncJsonResponse *response = prepareAlpacaResponse(request);
              JsonObject doc = response->getRoot();

              doc["Value"] = "4.0.0";

              response->setLength();
              request->send(response);
            });

  alpaca
      .on("/api/v1/switch/0/driverinfo", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = "Servo can be async now";

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpParams);

  alpaca
      .on("/api/v1/switch/0/interfaceversion", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = 3;

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpParams);

#pragma endregion

#pragma region AlpacaDevice

  alpaca
      .on("/api/v1/switch/0/maxswitch", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = Switches.getConfiguredSwitch();

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&modEN, &getAlpParams});

  alpaca
      .on("/api/v1/switch/0/canasync", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = true;

            request->send(response);
          })
      .addMiddlewares({&getAlpParams});

  alpaca
      .on("/api/v1/switch/0/canwrite", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] =
                Switches.canBeWrite(request->getAttribute("id").toInt());

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpParams, &checkID});

  alpaca
      .on("/api/v1/switch/0/getswitch", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] =
                Switches.getSwitchState(request->getAttribute("id").toInt())
                    ? true
                    : false;

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpParams, &checkID});

  alpaca
      .on("/api/v1/switch/0/getswitchname", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = Switches.getSwitchName(id);

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpParams, &checkID});

  alpaca
      .on("/api/v1/switch/0/getswitchdescription", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = Switches.getSwitchDescription(id);

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpParams, &checkID});

  alpaca
      .on("/api/v1/switch/0/getswitchvalue", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = Switches.getSwitchState(id);

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpParams, &checkID});

  alpaca
      .on("/api/v1/switch/0/minswitchvalue", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = Switches.getMin(id);

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpParams, &checkID});

  	alpaca.on("/api/v1/switch/0/maxswitchvalue", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        int id = request->getAttribute("id").toInt();

        doc["Value"] = Switches.getMax(id);

        response->setLength();
        request->send(response);
    }).addMiddlewares({&getAlpParams, &checkID});

  	alpaca.on("/api/v1/switch/0/switchstep", HTTP_GET, [](AsyncWebServerRequest *request) {
    	AsyncJsonResponse *response = prepareAlpacaResponse(request);
    	JsonObject doc = response->getRoot();

		doc["Value"] = 1;

    	response->setLength();
    	request->send(response);
    }).addMiddlewares({&getAlpParams});

	alpaca.on("/api/v1/switch/0/statechangecomplete", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        int id = request->getAttribute("id").toInt();

        doc["Value"] = true;

        response->setLength();
        request->send(response);
    }).addMiddlewares({&getAlpParams,&checkID});

  alpaca.on("/api/v1/switch/0/setswitch", HTTP_PUT, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse *response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    int id = request->getAttribute("id").toInt();
    int state = request->getAttribute("state").toInt();

    int value = 0;
    if (state) { value = Switches.getMax(id); }

    Switches.setSwitchState(id,state);

    response->setLength();
    request->send(response);
  }).addMiddlewares({&getAlpParams, &checkID, &checkState, &isSettable});
  
#pragma endregion
}
#endif