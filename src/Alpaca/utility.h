#ifndef ALPACA_UTILITY
#define ALPACA_UTILITY


AsyncMiddlewareFunction getAlpParams([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
  int paramsNr = request->params();
  String parameter;
  AlpacaData.serverTransactionID++;

  request->setAttribute("ServerTransactionID",  String(AlpacaData.serverTransactionID));
  request->setAttribute("ClientTransactionID",  static_cast<long>(0));
  for (int i = 0; i < paramsNr; i++) {
    const AsyncWebParameter* p = request->getParam(i);
    String name;
    name = p->name();
    name.toLowerCase();
    
    if (name == "clienttransactionid" || name == "id" || name == "value" || name == "brightness") {
        request->setAttribute(name.c_str(), p->value());
    }

	if(name == "state"){
		String State = p->value();
        State.toLowerCase();
		if(State == "true") {
			request->setAttribute(name.c_str(), (long)0);
		} else if (State == "false"){
			request->setAttribute(name.c_str(), (long)1);
		}
	}
  }
  
  next();
});



//This function prepare the response with error 0, client and server transiction id
AsyncJsonResponse* prepareAlpacaResponse(AsyncWebServerRequest *request) {
  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject doc = response->getRoot();

  //the library store parameters as string, i need to reconvert back to uint32
  doc["ClientTransactionID"] = strtoul(request->getAttribute("ClientTransactionID").c_str(), nullptr, 10);
  doc["ServerTransactionID"] = strtoul(request->getAttribute("ServerTransactionID").c_str(), nullptr, 10);
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

#endif