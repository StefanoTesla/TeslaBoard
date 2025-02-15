#ifndef CC_MAIN
#define CC_MAIN

PWMOutput Calibrator;
ServoOutput Cover;

#include "config.h"

void calibratorhandlerloop() {

    if(!CoverC.config.calibrator.present){
      CoverC.status.calibrator.status = CalibStatusNoPresent;
      return;
    } else {
      CoverC.status.calibrator.actualBrightness = Calibrator.status();
      if(Calibrator.status() == 0){
        CoverC.status.calibrator.status = CalibStatusOff;
      } else {
        CoverC.status.calibrator.status = CalibStatusReady;
      }
    }

    if(CoverC.command.calibrator.change){
        logMessageFormatted(coverc,lInfo,"Brightness changed to: %d", CoverC.command.calibrator.brightness);
        CoverC.command.calibrator.change = false;
        Calibrator.write(CoverC.command.calibrator.brightness);
    }
}

void coverCycle(){
  int increment;

  CoverC.status.cover.angle = Cover.status();
  
  if (Cover.isMoving()){
    CoverC.status.cover.status = CoverStatusMoving;
  } else {
    if(Cover.isClose()){
      CoverC.status.cover.status = CoverStatusClose;
    } else if (Cover.isOpen()){
      CoverC.status.cover.status = CoverStatusOpen;
    } else {
      CoverC.status.cover.status = CoverStatusUnknow;
    }
    
  }
  
  if( CoverC.status.cover.status != CoverStatusMoving ){ return; }

  if(CoverC.command.cover.move){
    if(CoverC.status.cover.status == CoverStatusUnknow){
      Cover.write(CoverC.command.cover.angle);
    } else {
      Cover.goToSlowly(CoverC.command.cover.angle);
    }
    CoverC.command.cover.move = false;
  }

  Cover.loop();
  
}

void coverHandlerloop() {

    if(!CoverC.config.cover.present){
      CoverC.status.cover.status = CoverStatusNoPresent;
      return;
    }
    
    coverCycle();
    
}



void coverCalibratorLoop(){
  calibratorhandlerloop();
  coverHandlerloop();

  if (CoverC.config.save.execute){
    saveCoverCConfig();
  }
}

#include "webserver.h"
#include "alpacaDevice.h"
#include "alpacaManage.h"

void coverCalibratorRequestHandler(){

  coverAlpacaManage();
  coverAlpacaDevice();
  coverWebServer();
}
#endif