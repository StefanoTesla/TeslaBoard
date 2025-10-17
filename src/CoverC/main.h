#ifndef CC_MAIN
#define CC_MAIN

#include "api.h"
#include "alpaca.h"

AsyncMiddlewareFunction isCoverCEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(CoverCalibrator.isEnable()){
      next();
    } else {
      request->send(500, "text/plain", "Module not enabled");
    }
    
});
AsyncMiddlewareFunction isCoverEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(CoverCalibrator.cover.isEnable()){
      next();
    } else {
      request->send(500, "text/plain", "Module not enabled");
    }
    
});
AsyncMiddlewareFunction isCalibratorEnable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    if(CoverCalibrator.calibrator.isEnable()){
      next();
    } else {
      request->send(500, "text/plain", "Module not enabled");
    }
    
});


void coverCalibratorRequestHandler(){

  coverAlpacaManage();
  coverAlpacaDevice();
  coverWebApi();
}
#endif