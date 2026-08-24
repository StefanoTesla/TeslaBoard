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
  }).addMiddleware(&getAlpParams);

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
  }).addMiddleware(&getAlpParams);

  alpaca.on("/management/v1/configureddevices",          HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    JsonArray Value = doc["Value"].to<JsonArray>();

    uint64_t chipMac = ESP.getEfuseMac();
    uint8_t mac[6];
    for (int i = 0; i < 6; i++) {
      mac[i] = (chipMac >> (i * 8)) & 0xFF;
    }
    char macPart[9];
    snprintf(macPart, sizeof(macPart), "%02x%02x%02x%02x", mac[2], mac[3], mac[4], mac[5]);
    char fullID[46]; 

    if(Dome.isEnable()){
      JsonObject dome = Value.add<JsonObject>();
      dome["DeviceName"] = Dome.getIdentifier() + " - " + Board.getIdentifier();
      dome["DeviceType"] = "Dome";
      dome["DeviceNumber"] = 0;
      snprintf(fullID, sizeof(fullID), "%s-ba0f-4834-b299-79a629f2ee59", macPart);
      dome["UniqueID"] = fullID;
    }

    if(Switches.isEnable()){
      JsonObject switc = Value.add<JsonObject>();
      switc["DeviceName"] = Switches.getIdentifier() + " - " + Board.getIdentifier();
      switc["DeviceType"] = "Switch";
      switc["DeviceNumber"] = 0;
      snprintf(fullID, sizeof(fullID), "%s-aa85-49ed-8799-9f50c0969ede", macPart);
      switc["UniqueID"] = fullID;
    }

    if(CoverCalibrator.isEnable()){
      JsonObject cover = Value.add<JsonObject>();
      cover["DeviceName"] = CoverCalibrator.getIdentifier() + " - " + Board.getIdentifier();
      cover["DeviceType"] = "CoverCalibrator";
      cover["DeviceNumber"] = 0;
      snprintf(fullID, sizeof(fullID), "%s-4862-4947-a8f5-196b2ee72529", macPart);
      cover["UniqueID"] = fullID;
    }
    
    response->setLength();
    request->send(response);
  }).addMiddleware(&getAlpParams);

}


#endif