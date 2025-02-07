#include <Arduino.h>
#include "ServoOutput.h"

ServoOutput::ServoOutput(){
    
}

void ServoOutput::setup(int _pin, int _channel){
    pin = _pin;
    channel = _channel;
    ledcAttachPin(pin,channel);
}

int ServoOutput::write(int _value) {
    int dutyMicros = map(_value, 0, maxDeg, 544, 2500);
    int dutyValue = map(dutyMicros, 0, 20000, 0, 4095); 
    ledcWrite(channel, dutyValue);
}

int ServoOutput::readPin() {
    return ledcRead(channel);
}

int ServoOutput::status(){
    float angle = ((float)(readPin() - 111) / (511 - 111)) * maxDeg;
    return round(angle);
}

unsigned int ServoOutput::getChannel(){
    return channel;
}

int ServoOutput::getType(){
    return 4;
}

bool ServoOutput::isClose(){
    if(status()==closeDeg && !moving){
        return true;
    }
    return false;
}
bool ServoOutput::isOpen(){
    if(status()==openDeg && !moving){
        return true;
    }
    return false;
}

bool ServoOutput::isMoving(){
    if(moving){
        return true;
    }
    return false;
}


void ServoOutput::slowMove(int _angle){


}

void ServoOutput::loop(){


}


/*

switch (CoverC.status.cover.cycle){
    case 0:
      if (CoverC.status.cover.angle == CoverC.config.cover.closeDeg){
        CoverC.status.cover.status = CoverStatusClose;
      } else if (CoverC.status.cover.angle == CoverC.config.cover.openDeg){
        CoverC.status.cover.status = CoverStatusOpen;
      } else if( CoverC.status.cover.angle < 0 or CoverC.status.cover.angle > CoverC.config.cover.maxDeg){
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
      
      Cover.write(CoverC.command.cover.angle);
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
      if(CoverC.command.cover.angle > CoverC.status.cover.angle){
        logMessage(coverc,lInfo,"Cy:20 Moving to an higher position");
        CoverC.command.cover.handler.inc = true;
        
      } else if(CoverC.command.cover.angle < CoverC.status.cover.angle ){
        logMessage(coverc,lInfo,"Cy:20 Moving to a lower position");
        CoverC.command.cover.handler.inc = false;
      }

      // do the magic
      CoverC.status.cover.cycle = 30;


    }

      break;

    case 30:
    if (Global.actualMillis - CoverC.command.cover.handler.ackMillis > CoverC.command.cover.handler.stepTime){

      if(CoverC.command.cover.handler.inc){
        CoverC.command.cover.handler.angle++;
        CoverC.command.cover.handler.ackMillis = Global.actualMillis;
        if(CoverC.status.cover.angle >= CoverC.command.cover.angle){
          Cover.write(CoverC.command.cover.angle);
          CoverC.status.cover.cycle = 0;
          CoverC.command.cover.move = false;
          logMessage(coverc,lInfo,"Cy:30 finish");
        } else {
          Cover.write(CoverC.command.cover.handler.angle);
        }
      } else {
        CoverC.command.cover.handler.angle--;
        if(CoverC.command.cover.handler.angle < 0){
          CoverC.command.cover.handler.angle = 0;
        }
        CoverC.command.cover.handler.ackMillis = Global.actualMillis;
        if(CoverC.status.cover.angle <= CoverC.command.cover.angle ){
          Cover.write(CoverC.command.cover.angle);
          CoverC.status.cover.cycle = 0;
          CoverC.command.cover.move = false;
          logMessage(coverc,lInfo,"Cy:30 finish");
        } else {
          Cover.write(CoverC.command.cover.handler.angle);
        }
      }

      
    }

      break;



    default:
      logMessage(coverc,lErr,"Cover cycle OverFlow");
      Serial.println("COVER: CYCLE OVERFLOW");

  }

*/