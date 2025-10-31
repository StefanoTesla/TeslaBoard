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