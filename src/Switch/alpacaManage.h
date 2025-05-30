#ifndef SWITCH_ALPACA_MANAGE
#define SWITCH_ALPACA_MANAGE



void switchAlpacaManage(){

  alpaca.on("/api/v1/switch/0/name",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      doc["Value"] = std::string(SWITCH_IDENTIFIER) + " - TeslaBoard";
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/switch/0/description",                                              HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      doc["Value"] = "Switch handle by the mitic StefanoTesla Board";
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/switch/0/driverversion",                                            HTTP_GET, [](AsyncWebServerRequest *request) {
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

  alpaca.on("/api/v1/switch/0/driverinfo",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      doc["Value"] = "Rolling back to more readable driver with middlware";
      doc["ErrorNumber"] = 0;
      doc["ErrorMessage"] = "";
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/switch/0/interfaceversion",                                               HTTP_GET, [](AsyncWebServerRequest *request) {
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