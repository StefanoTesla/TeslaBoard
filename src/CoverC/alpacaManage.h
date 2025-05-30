#ifndef CC_ALPACA_MANAGE
#define CC_ALPACA_MANAGE



void coverAlpacaManage(){

  alpaca.on("/api/v1/covercalibrator/0/name",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      doc["Value"] = std::string(CC_IDENTIFIER) + " - TeslaBoard";
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/covercalibrator/0/description",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      doc["Value"] = "CoverCalibrator handle by the mitic StefanoTesla Board";
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/covercalibrator/0/driverversion",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    doc["Value"] = "4.0.0";
    doc["ErrorNumber"] = 0;
    doc["ErrorMessage"] = "";
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
  });

  alpaca.on("/api/v1/covercalibrator/0/driverinfo",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      doc["Value"] = "rolling back to more readable driver with middlware";
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/covercalibrator/0/interfaceversion",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    doc["Value"] = 2;
    doc["ErrorNumber"] = 0;
    doc["ErrorMessage"] = "";
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);
}
#endif