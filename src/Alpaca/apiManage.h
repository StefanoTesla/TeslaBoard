#ifndef ALPACA_MANAGE
#define ALPACA_MANAGE

void AlpacaManager(){

  alpaca.on("/management/apiversions",                HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    JsonArray Value = doc["Value"].to<JsonArray>();
    Value.add(1);
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/management/v1/description",                HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    JsonObject Value = doc["Value"].to<JsonObject>();
    Value["ServerName"] = "TeslaBoard Alpaca Device";
    Value["Manufacturer"] = "Stefano Tesla";
    Value["ManufacturerVersion"] = "v4.0.0";
    Value["Location"] = "Empoli, IT";
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/management/v1/configureddevices",          HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    JsonArray Value = doc["Value"].to<JsonArray>();

    #ifdef DOME
      JsonObject dome = Value.add<JsonObject>();
      dome["DeviceName"] = std::string(DOME_IDENTIFIER) + " - TeslaBoard";
      dome["DeviceType"] = "Dome";
      dome["DeviceNumber"] = 0;
      dome["UniqueID"] = "e989c9b6-ba0f-4834-b299-79a629f2ee59";
    #endif

    #ifdef SWITCH
      JsonObject switc = Value.add<JsonObject>();
      switc["DeviceName"] = std::string(SWITCH_IDENTIFIER) + " - TeslaBoard";
      switc["DeviceType"] = "Switch";
      switc["DeviceNumber"] = 0;
      switc["UniqueID"] = "d93f20fb-aa85-49ed-8799-9f50c0969ede";
    #endif

    #ifdef COVER_CALIBRATOR
      JsonObject cover = Value.add<JsonObject>();
      cover["DeviceName"] = std::string(CC_IDENTIFIER) +" - TeslaBoard";
      cover["DeviceType"] = "CoverCalibrator";
      cover["DeviceNumber"] = 0;
      cover["UniqueID"] = "35672690-40bf-4165-b44e-d59c2c524f11";
    #endif
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);

}


#endif