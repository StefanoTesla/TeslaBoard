#ifndef SWITCH_ALPACA_MANAGE
#define SWITCH_ALPACA_MANAGE



void switchAlpacaManage(){

  alpaca.on("/api/v1/switch/0/name",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      
      doc["Value"] = std::string(SWITCH_IDENTIFIER) + " - TeslaBoard";
      
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/switch/0/description",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();

      doc["Value"] = "Switch handled by Stefano TeslaBoard";
      
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/switch/0/driverversion",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();

    doc["Value"] = SW_VERSION;

    response->setLength();
    request->send(response);
  });

  alpaca.on("/api/v1/switch/0/driverinfo",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();

      doc["Value"] = "New IOManager";

      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/switch/0/interfaceversion",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();

    doc["Value"] = 3;
    
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);
}
#endif