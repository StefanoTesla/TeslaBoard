#ifndef DOME_ALPACA_MANAGE
#define DOME_ALPACA_MANAGE



void domeAlpacaManage(){

  alpaca.on("/api/v1/dome/0/name",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      doc["Value"] = std::string(DOME_IDENTIFIER) + " - TeslaBoard";
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/dome/0/description",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      doc["Value"] = "Dome handler R.O.R. by the mitic StefanoTesla Board";
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/dome/0/driverversion",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
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

  alpaca.on("/api/v1/dome/0/driverinfo",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
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

  alpaca.on("/api/v1/dome/0/interfaceversion",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    doc["Value"] = 3;
    doc["ErrorNumber"] = 0;
    doc["ErrorMessage"] = "";
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);
}
#endif