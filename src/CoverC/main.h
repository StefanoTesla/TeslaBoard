#ifndef CC_MAIN
#define CC_MAIN

#include "config.h"

void calibratorhandlerloop() {

    if(!CoverC.config.calibrator.present){
      CoverC.status.calibrator.status = CalibStatusNoPresent; 
    } else {
      CoverC.status.calibrator.actualBrightness = ledcRead(CoverC.config.calibrator.pwmChannel);
      if(ledcRead(CoverC.config.calibrator.pwmChannel) == 0){
        CoverC.status.calibrator.status = CalibStatusOff;
      } else {
        CoverC.status.calibrator.status = CalibStatusReady;
      }
    }

    if(CoverC.command.calibrator.change){
        logMessageFormatted(coverc,lInfo,"Brightness changed to: %d", CoverC.command.calibrator.brightness);
        CoverC.command.calibrator.change = false;
        Serial.println(CoverC.command.calibrator.brightness);
        ledcWrite(CoverC.config.calibrator.pwmChannel,CoverC.command.calibrator.brightness);
    }
}

void setServoAngle(int angle) {
  //logMessageFormatted(coverc,lInfo,"Cover moved to: %d", angle);
  int dutyMicros = map(angle, 0, CoverC.config.cover.maxDeg, 544, 2500);
  int dutyValue = map(dutyMicros, 0, 20000, 0, 4095); 
  ledcWrite(CoverC.config.cover.pwmChannel, dutyValue);
}


int dutyToAngle(int duty) {
  float angle = ((float)(duty - 111) / (511 - 111)) * CoverC.config.cover.maxDeg;
  return round(angle);  
}

void coverCycle(){
  int increment;
  CoverC.status.cover.angle = dutyToAngle(ledcRead(CoverC.config.cover.pwmChannel));
    
  switch (CoverC.status.cover.cycle){
    case 0:
      if (CoverC.status.cover.angle == CoverC.config.cover.closeDeg){
        CoverC.status.cover.status = CoverStatusClose;
      } else if (CoverC.status.cover.angle == CoverC.config.cover.openDeg){
        CoverC.status.cover.status = CoverStatusOpen;
      }
      if( CoverC.status.cover.angle < 0 or CoverC.status.cover.angle > CoverC.config.cover.maxDeg){
        CoverC.status.cover.status = CoverStatusUnknow;
      }
      
      if(CoverC.status.cover.status != CoverStatusMoving){
        if (CoverC.command.cover.move){
          CoverC.command.cover.handler.ackMillis = Global.actualMillis;
          if(CoverC.status.cover.status == CoverStatusUnknow){
            CoverC.status.cover.cycle = 10;
          } else {
            CoverC.status.cover.cycle = 20;
          }
          CoverC.status.cover.status = CoverStatusMoving;
        }
      }

      break;
    
    //from unknow angle
    case 10:
      logMessage(coverc,lInfo,"Cy:10 Moving from undefined position");
      
      setServoAngle(CoverC.command.cover.angle);
      CoverC.status.cover.cycle = 11;
      break;
    
    case 11:
      if((Global.actualMillis - CoverC.command.cover.handler.ackMillis) > CoverC.config.cover.movingTime){
        logMessage(coverc,lInfo,"Cy:11 Moviment finish");
        CoverC.command.cover.move = false;
        CoverC.status.cover.cycle = 0;
      }
      break;

    //inc or dec angle?
    case 20:
    if(CoverC.command.cover.angle == CoverC.status.cover.angle){
      CoverC.status.cover.cycle = 0;
      CoverC.command.cover.move = false;
    } else {
      // get in how many ms I need to do a degree
      CoverC.command.cover.handler.stepTime = CoverC.config.cover.movingTime / CoverC.config.cover.maxDeg;
      if( CoverC.command.cover.handler.stepTime == 0) {
        CoverC.command.cover.handler.stepTime = 1;
      }
      logMessageFormatted(coverc,lInfo,"Cy:20 Step time: %d", CoverC.command.cover.handler.stepTime);
      logMessageFormatted(coverc,lInfo,"Cy:20 Moving time: %d", CoverC.config.cover.movingTime);

      // check if I need to encrease or decrease
      CoverC.command.cover.handler.inc = CoverC.status.cover.angle < CoverC.command.cover.angle ? true : false;
      if(CoverC.command.cover.handler.inc){
        logMessage(coverc,lInfo,"Cy:20 Moving to an higher position");
      } else{
        logMessage(coverc,lInfo,"Cy:20 Moving to a lower position");
      }

      // do the magic
      CoverC.status.cover.cycle = 30;


    }

      break;

    case 30:
      if (Global.actualMillis - CoverC.command.cover.handler.ackMillis > CoverC.command.cover.handler.stepTime){
        CoverC.command.cover.handler.angle += CoverC.command.cover.handler.inc ? 1 : -1;

        CoverC.command.cover.handler.ackMillis = Global.actualMillis;

        if((CoverC.command.cover.handler.inc && CoverC.status.cover.angle < CoverC.command.cover.angle) ||
            (!CoverC.command.cover.handler.inc && CoverC.status.cover.angle >= CoverC.command.cover.angle))
            {
              setServoAngle(CoverC.command.cover.handler.angle);
          } else { 
            setServoAngle(CoverC.command.cover.angle);
            CoverC.status.cover.cycle = 0;
            CoverC.command.cover.move = false;
            logMessage(coverc,lInfo,"Cy:30 finish");
          }
      }
      break;

    default:
      logMessage(coverc,lErr,"Cover cycle OverFlow");
      Serial.println("COVER: CYCLE OVERFLOW");

  }

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