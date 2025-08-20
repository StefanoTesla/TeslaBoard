#ifndef DOME_ALPACA_MANAGE
#define DOME_ALPACA_MANAGE



void domeAlpacaManage(){

  alpaca.on("/api/v1/dome/0/name",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();

      doc["Value"] = std::string(DOME_IDENTIFIER) + " - TeslaBoard";

      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/dome/0/description",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();

      doc["Value"] = "Dome handler R.O.R. by Stefano TeslaBoard";

      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/dome/0/driverversion",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();

    doc["Value"] = SW_VERSION;
    
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/dome/0/driverinfo",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();

      doc["Value"] = "Using the IOManager and three way to handle r.o.r.";

      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/dome/0/interfaceversion",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();

    doc["Value"] = 3;

    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);
}
#endif