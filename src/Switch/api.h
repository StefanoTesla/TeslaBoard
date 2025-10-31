#ifndef SWITCH_WEBSERVER
#define SWITCH_WEBSERVER

extern SwitchModule Switches;

void webApi() {

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

  server.on(
      "/api/switch/set-value", HTTP_POST, [](AsyncWebServerRequest *request) {
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

        if (id < 0 || id >= Switches.getConfiguredSwitch()) {
          doc["error"] = "SwIdOutOfRange";
          response->setLength();
          request->send(response);
          return;
        }

        if (Switches.isWritable(id))
          ;

        if (value < 0) {
          doc["error"] = "SwValueAbsent";
          response->setLength();
          request->send(response);
          return;
        } else {
          if (value < Switches.getMin(id)) {
            doc["error"] = "SwValueBehindMin";
            response->setLength();
            request->send(response);
            return;
          }
          if (value > Switches.getMax(id)) {
            doc["error"] = "SwValueOverMax";
            response->setLength();
            request->send(response);
            return;
          }
        }

        Switches.setSwitchState(id,value);

        doc["execute"] = true;

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
}
#endif