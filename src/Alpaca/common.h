#ifndef ALPACA_COMMON_RESPONSE
#define ALPACA_COMMON_RESPONSE

//This function prepare the response with error 0, client and server transiction id
AsyncJsonResponse* prepareAlpacaResponse(AsyncWebServerRequest *request) {
  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject doc = response->getRoot();

  doc["ClientTransactionID"] = request->getAttribute("ClientTransactionID").toInt();
  doc["ServerTransactionID"] = request->getAttribute("ServerTransactionID").toInt();
  doc["ErrorNumber"] = 0;
  doc["ErrorMessage"] = "";
  return response;
}
/*
Response to a Method not implemented from the board
*/
void alpacaMethodNotImplemented(AsyncWebServerRequest *request){
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    doc["ErrorNumber"] = 1024;
    doc["ErrorMessage"] = "Method not implemented";
    response->setLength();
    request->send(response);
}
/*
Response to a Property not implemented from the board
*/
void alpacaPropertyNotImplemented(AsyncWebServerRequest *request){
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    doc["ErrorNumber"] = 1024;
    doc["ErrorMessage"] = "Property not implemented";
    response->setLength();
    request->send(response);
}


/*
Response to a can do something not implemented from the board
*/
void alpacaCant(AsyncWebServerRequest *request){
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    doc["Value"] = false;
    response->setLength();
    request->send(response);
}


/*
Response to action list request but any actions are implemented
*/
void alpacaNoActions(AsyncWebServerRequest *request){
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    JsonArray Value = doc["Value"].to<JsonArray>();
    response->setLength();
    request->send(response);
}

void alpacaActionNotImplemented(AsyncWebServerRequest *request){
    AsyncJsonResponse* response = prepareAlpacaResponse(request);
    JsonObject doc = response->getRoot();
    doc["ErrorNumber"] = 1036;
    doc["ErrorMessage"] = "Action not implemented";
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


#endif