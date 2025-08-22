#ifndef ALPACA_MIDDLEWARE
#define ALPACA_MIDDLEWARE



AsyncMiddlewareFunction getAlpacaID([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
  int paramsNr = request->params();
  String parameter;
  AlpacaData.serverTransactionID++;

  request->setAttribute("serverTransactionID",  AlpacaData.serverTransactionID);
  request->setAttribute("clientTransictionID",  "0");
  for (int i = 0; i < paramsNr; i++) {
    const AsyncWebParameter* p = request->getParam(i);
    parameter = p->name();
    parameter.toLowerCase();
    if (parameter == "clienttransactionid") {
      request->setAttribute("clientTransictionID",  p->value());
    }    
  }
  
  next();
});

#endif