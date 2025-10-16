#ifndef ALPACA_MANAGE
#define ALPACA_MANAGE

void AlpacaManager(){

  alpaca.on("/management/apiversions",                HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    JsonArray Value = doc["Value"].to<JsonArray>();
    Value.add(1);
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/management/v1/description",                HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    JsonObject Value = doc["Value"].to<JsonObject>();
    Value["ServerName"] = "TeslaBoard Alpaca Device";
    Value["Manufacturer"] = "Stefano Tesla";
    Value["ManufacturerVersion"] = SW_VERSION;
    Value["Location"] = "Empoli, IT";
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);

  alpaca.on("/management/v1/configureddevices",          HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    JsonArray Value = doc["Value"].to<JsonArray>();

    if(Dome.isEnable()){
      JsonObject dome = Value.add<JsonObject>();
      dome["DeviceName"] = std::string(DOME_IDENTIFIER) + " - TeslaBoard";
      dome["DeviceType"] = "Dome";
      dome["DeviceNumber"] = 0;
      dome["UniqueID"] = "e989c9b6-ba0f-4834-b299-79a629f2ee59";
    }

    if(Switch.config.isEnable){
      JsonObject switc = Value.add<JsonObject>();
      switc["DeviceName"] = std::string(SWITCH_IDENTIFIER) + " - TeslaBoard";
      switc["DeviceType"] = "Switch";
      switc["DeviceNumber"] = 0;
      switc["UniqueID"] = "d93f20fb-aa85-49ed-8799-9f50c0969ede";
    }

    if(CoverC.config.isEnable){
    JsonObject cover = Value.add<JsonObject>();
    cover["DeviceName"] = std::string(CC_IDENTIFIER) +" - TeslaBoard";
    cover["DeviceType"] = "CoverCalibrator";
    cover["DeviceNumber"] = 0;
    cover["UniqueID"] = "35672690-40bf-4165-b44e-d59c2c524f11";
    }
    
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpacaID);

}


#endif