#ifndef DOME_MAIN
#define DOME_MAIN
extern DomeModule Dome;

AsyncMiddlewareFunction isDomeEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(Dome.isEnable()){
      next();
    } else {
      request->send(500, "text/plain", "Module not enabled");
    }
    
});


#include "api.h"
#include "alpaca.h"

void domeRequestHandler(){
  domeAlpacaDevices();
  domeAlpacaManage();
  domeWebApi();
}
#endif