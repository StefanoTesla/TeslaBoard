#ifndef ALPACA_UTILITY
#define ALPACA_UTILITY


AsyncMiddlewareFunction getAlpParams([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
  int paramsNr = request->params();
  String parameter;
  AlpacaData.serverTransactionID++;
  static char stidBuffer[12];
    sprintf(stidBuffer, "%lu", AlpacaData.serverTransactionID);
  request->setAttribute("stid",  stidBuffer);
  request->setAttribute("ctid",  static_cast<long>(0));
  for (int i = 0; i < paramsNr; i++) {
    const AsyncWebParameter* p = request->getParam(i);
    String name;
    name = p->name();
    name.toLowerCase();

    //since the parameter name pointer goes away, I have to hard write any parameter name :(
    if(name=="id"){ request->setAttribute("id", p->value().c_str()); continue; }
    if(name == "clienttransactionid"){ request->setAttribute("ctid", p->value().c_str()); continue; }
    if(name == "value"){ request->setAttribute("value", p->value().c_str()); continue; }
    if(name == "brightness") { 

      Serial.println(p->value());
      request->setAttribute("brightness", p->value().c_str()); continue; }

	if(name == "state"){
		String State = p->value();
    State.toLowerCase();
		if(State == "true") {
			request->setAttribute("state", (long)1);
		} else if (State == "false"){
			request->setAttribute("state", (long)0);
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
  doc["ClientTransactionID"] = strtoul(request->getAttribute("ctid").c_str(), nullptr, 10);
  doc["ServerTransactionID"] = strtoul(request->getAttribute("stid").c_str(), nullptr, 10);
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