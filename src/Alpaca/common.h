#ifndef ALPACA_COMMON_RESPONSE
#define ALPACA_COMMON_RESPONSE

/*
Response to a Method not implemented from the board
*/
void alpacaMethodNotImplemented(AsyncWebServerRequest *request){
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    doc["ErrorNumber"] = 1024;
    doc["ErrorMessage"] = "Method not implemented";
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
}
/*
Response to a Property not implemented from the board
*/
void alpacaPropertyNotImplemented(AsyncWebServerRequest *request){
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    doc["ErrorNumber"] = 1024;
    doc["ErrorMessage"] = "Property not implemented";
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
}

/*
Response to a can do something implemented from the board
*/
void alpacaCan(AsyncWebServerRequest *request){
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    doc["Value"] = true;
    doc["ErrorNumber"] = 0;
    doc["ErrorMessage"] = "";
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
}

/*
Response to a can do something not implemented from the board
*/
void alpacaCant(AsyncWebServerRequest *request){
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    doc["Value"] = false;
    doc["ErrorNumber"] = 0;
    doc["ErrorMessage"] = "";
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
}


/*
Response to action list request but any actions are implemented
*/
void alpacaNoActions(AsyncWebServerRequest *request){
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    JsonArray Value = doc["Value"].to<JsonArray>();
    doc["ErrorNumber"] = 0;
    doc["ErrorMessage"] = "";
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
}

void alpacaActionNotImplemented(AsyncWebServerRequest *request){
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    doc["ErrorNumber"] = 1036;
    doc["ErrorMessage"] = "Action not implemented";
    doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
    doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
    response->setLength();
    request->send(response);
}


/*
*
* Validate Pin Number as Input
*
*/

bool commonValidateInputPin(unsigned int pin){
    int unusable[15] = {1, 6, 7, 8, 9, 10, 11,20,24,28,29,30,31,37,38};
    int i =0;
    if(
        pin < 0 and
        pin > 39
    ){
        return false;
    }

    // check if I can't use this pin
    for(i=0;i<7;i++){

        if(pin == unusable[i]){
            return false;
        }
    }

    return true;

}
bool commonValidateOutputPin(unsigned int pin){
    int unusable[15] = {3,6,7,8,9,10,11,20,24,28,29,30,31,37,38};
    int i =0;
    if(
        pin < 0 and
        pin > 33
    ){
        return false;
    }

    // check if I can't use this pin
    for(i=0;i<7;i++){

        if(pin == unusable[i]){
            return false;
        }
    }

    return true;

}

//This function prepare the response with error 0, client and server transiction id
AsyncJsonResponse* prepareAlpacaResponse() {
  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject doc = response->getRoot().to<JsonObject>();

  doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
  doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
  doc["ErrorNumber"] = 0;
  doc["ErrorMessage"] = "";
  return response;
}
#endif