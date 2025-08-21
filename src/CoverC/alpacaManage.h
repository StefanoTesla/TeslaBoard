#ifndef CC_ALPACA_MANAGE
#define CC_ALPACA_MANAGE



void coverAlpacaManage(){

  alpaca.on("/api/v1/covercalibrator/0/name",                                                     HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();
      doc["Value"] = std::string(CC_IDENTIFIER) + " - TeslaBoard";
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/covercalibrator/0/description",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();
      doc["Value"] = "CoverCalibrator by Stefano TeslaBoard";
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/covercalibrator/0/driverversion",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    doc["Value"] = SW_VERSION;
    response->setLength();
    request->send(response);
  });

  alpaca.on("/api/v1/covercalibrator/0/driverinfo",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = "New version with 20kHz flat panel and cover support";
    
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/covercalibrator/0/interfaceversion",                                         HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();

    doc["Value"] = 2;

    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);
}
#endif