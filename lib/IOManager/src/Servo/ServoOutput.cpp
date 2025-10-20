#include <Arduino.h>
#include "ServoOutput.h"
#include "IOConfigStruct.h"
//This library is totally outside the Servo.h arduino library
//I handle the servo like a PWM output and I calcolate the duty in microsecond with a 12bit resolution.
//Since I will use slow timer, I declare that 1° of minimum moviment can be fine
//Going down means fight with float values and I don't want to fight, I'm pacific person.
//I assume olso that minimum position is 0°
//I can't go below zero, or everything here should be rewritte

// this library assume olso that the used servo following the standard servo specific 50hz 0°=544us full rotation°=2500us
// if you need to change it:
// check the write function where 544 and 2500 are the microsecond for 0 and full rotation.
// check the status function: 111 is the ledcRead value when rotation is 0° and 511 when servo is fully rotated.

//the Slow Moviment got an auxiliar parameter _overridePosition, if set to true, status will return the destination position instead the real angle
//this to bypass a problem with N.I.N.A

void ServoOutput::setup(IOConfigBase* config){
  if (config->getType() != 4) {
    Serial.println("Errore: SERVO tipo di configurazione non valido!");
    return;
  }

  ServoOutputConfig* cfg = static_cast<ServoOutputConfig*>(config);
  ledcSetup(cfg->ledChannel, 50, 12);
  ledcAttachPin(cfg->pin,cfg->ledChannel); 

  pin = cfg->pin;
  min = 0;
  max = cfg->maxDeg;
  openDeg = cfg->openDeg;
  closeDeg = cfg->closeDeg;
  movingTime = cfg->movTime * 1000;

}


void ServoOutput::jsonSetup(JsonObject setup){

  channel = -1;
  channel = chMgr->getSlowChannel();

  if(channel > 0){
    pin = setup["pin"].as<unsigned int>();
    min = 0;
    max = setup["maxDeg"].as<unsigned int>();
    openDeg = setup["openDeg"].as<unsigned int>();
    closeDeg = setup["closeDeg"].as<unsigned int>();
    movingTime = setup["moveDeg"].as<unsigned int>() * 1000;
    ledcSetup(pin, 50, 12);
    ledcAttachPin(pin,channel); 
  }


}

bool ServoOutput::pinUnusable(int pin){
    if(pin == 1 or pin == 3 or (pin >=6 and pin <=11) or pin == 20 or pin == 24 or (pin >=28 and pin <= 31)){
        return true;
    }
    
    if(pin > 33){
        return true;
    }
  return false;
}


int ServoOutput::write(int _angle) {
    if (_angle >= 0 && _angle <= max){
      int dutyMicros = map(_angle, 0, 100, 544, 2500);
      int dutyValue = map(dutyMicros, 0, 20000, 0, 4095); 
      ledcWrite(pin, dutyValue);
      return 1;
    }
    return 0;

}

int ServoOutput::readPin() {
    return ledcRead(pin);
}

int ServoOutput::readAngle(){
  float angle = ((float)(readPin() - 111) / (511 - 111)) * max;
  return round(angle);
}

int ServoOutput::status(){
    if(overridePosition){
      currentAngle = MoveToSlowly.destination;
    } else {
      currentAngle = readAngle();
    }

    servoHandler();
    return currentAngle;

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


void ServoOutput::halt(){
  moving = false;
}

void ServoOutput::goTo(int _angle,bool slowPermitted){

  if(isReferenced() && slowPermitted && movingTime != 0){
    goToSlowly(_angle,true);
  } else {
    write(_angle);
  }
}


bool ServoOutput::goToSlowly(int _angle, bool _overridePosition){

      if(moving){
         return false;
      }

      if(readAngle() < 0 || readAngle()> max){
        Serial.println("Servo is in an undefined position, use write function");
        return false;
      }

      if(_angle < 0 || _angle > max){
        Serial.println("invalid commanded angle.");
        return false;
      }

      // get in how many ms I need to do a degree
      MoveToSlowly.destination = _angle;
      MoveToSlowly.intervall = movingTime / max;
      if(MoveToSlowly.intervall == 0){
        MoveToSlowly.intervall = 1;
      }
      MoveToSlowly.increment = readAngle() > _angle ? false : true; 
      moving = true;
      overridePosition = _overridePosition ? true : false;
      return true;
}

void ServoOutput::servoHandler(){

  if(!moving){
    return;
  }

  switch (cycle){
      case 0:
        if(!moving){
          overridePosition = false;
          return;
        }
        MoveToSlowly.actualMillis = millis();
        MoveToSlowly.startTime = millis();
        MoveToSlowly.nextDeg = readAngle();
        cycle = 10;
        break;

      case 10:
        if(millis() - MoveToSlowly.actualMillis >= MoveToSlowly.intervall){
          MoveToSlowly.nextDeg += MoveToSlowly.increment ? +1 : -1;

          if((MoveToSlowly.increment && MoveToSlowly.nextDeg < MoveToSlowly.destination) ||
              (!MoveToSlowly.increment && MoveToSlowly.nextDeg > MoveToSlowly.destination)) {
                write(MoveToSlowly.nextDeg);
                MoveToSlowly.actualMillis = millis();
          } else {
            write(MoveToSlowly.destination);
            cycle = 0;
            moving = false;
          }
        }
        break;
      
      default:
        cycle = 0;
        moving = false;
        break;
    }
  }

void ServoOutput::loop(){
}

void ServoOutput::setMax(int _value){
  if(_value > 0 && _value <= 360){
    max = _value;
  }
}

bool ServoOutput::isReferenced(){
  if(readAngle() >= 0 && readAngle()<=getMax()){
    return true;
  }
  return false;
}


int ServoOutput::validateJsonCfg(JsonObject json){
/*
return code table:
1 validation is ok
-1: pin is not unsigned integer
-10: pin is not asable as output
-5:maxServo not unsigned integer
-500: maxServo is out of range
-6:openDeg not unsigned integer
-600: openDeg is out of range
-601: openDeg is bigger than maxDeg
-7:closeDeg not unsigned integer
-700: closeDeg is out of range
-701: closeDeg is bigger than maxDeg
-8:movingTime not unsigned integer
-10: type is not unsigned integer
-1010: wrong type passed
*/
  int maxDeg = 0;
  int openDeg = 0;
  int closeDeg = 0;

  if(!json["pin"].is<unsigned int>()){
      return -1;
    } else {
      if(pinUnusable(json["pin"].as<unsigned int>())){
        return -10;
      }
  }

  if(!json["maxDeg"].is<unsigned int>()){
      return -5;
    } else {
      maxDeg = json["maxDeg"].as<unsigned int>();
      if(maxDeg>360){
        return -500;
      }
  }
  if(!json["openDeg"].is<unsigned int>()){
      return -6;
    } else {
      openDeg = json["openDeg"].as<unsigned int>();
      if(openDeg>360){
        return -600;
      }
      if(openDeg>maxDeg){
        return -601;
      }
  }
  
  if(!json["closeDeg"].is<unsigned int>()){
      return -7;
    } else {
      closeDeg = json["closeDeg"].as<unsigned int>();
      if(closeDeg>360){
        return -700;
      }
      if(closeDeg>maxDeg){
        return -701;
      }
  }

  if(!json["movTime"].is<unsigned int>()){
      return -8;
  }

  return 1;

}

void ServoOutput::getConfiguration(JsonObject cfg){
    cfg["pin"] = pin;
    cfg["maxDeg"] = max;
    cfg["openDeg"] = openDeg;
    cfg["closeDeg"] = closeDeg;
    cfg["movTime"] = movingTime;

}


void ServoOutput::handleMovement(){

  if(isMoving()==false){
    return;
  }

}