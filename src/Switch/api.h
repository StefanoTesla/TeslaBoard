#ifndef SWITCH_WEBSERVER
#define SWITCH_WEBSERVER

extern SwitchModule Switches;

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
              int id = NULL;
              int value = NULL;
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

              if (id == NULL) {
                doc["error"] = "SwIdOAbsent";
                response->setLength();
                request->send(response);
                return;
              }

              if (value == NULL) {
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
      .addMiddleware(&getAlpacaID);

  alpaca
      .on("/api/v1/switch/0/description", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = "Switch handled by Stefano TeslaBoard";

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/switch/0/driverversion", HTTP_GET,
            [](AsyncWebServerRequest *request) {
              AsyncJsonResponse *response = prepareAlpacaResponse(request);
              JsonObject doc = response->getRoot();

              doc["Value"] = "4.0.0";

              response->setLength();
              request->send(response);
            });

  alpaca.on("/api/v1/switch/0/driverinfo", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = "Servo cna be async now";

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/switch/0/interfaceversion", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = 3;

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpacaID);
#pragma endregion
}
#endif