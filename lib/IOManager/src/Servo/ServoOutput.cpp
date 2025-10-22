#include <Arduino.h>
#include "ServoOutput.h"
#include "IOConfigStruct.h"
#include "esp_log.h"
#define LOG_TAG "IOServo"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)
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

int ServoOutput::write(int _pos) {

    if (_pos >= 0 && _pos <= 100){
      int dutyMicros = map(_pos, 0, 100, 544, 2500);
      int dutyValue = map(dutyMicros, 0, 20000, 0, 4095); 
      LOGV("New value requested: %d, dutyValue: %d, channel: %d",_pos,dutyValue,channel);
      ledcWrite(channel, dutyValue);
      return 1;
    }
    return 0;

}

int ServoOutput::readPin() {
    return ledcRead(channel);
}

//Return a value from 0 to 100
//can return a negative value if servo was nevere moved
int ServoOutput::readPosition(){
  float angle = ((float)(readPin() - 111) / (511 - 111)) * 100;
  return round(angle);
}

int ServoOutput::status(){
  return overridePosition ? moveTo.destination : readPosition();
}

int ServoOutput::getType(){
    return 4;
}

void ServoOutput::halt(){
  positioning = false;
}


// Sets the servo movement duration and computes the update interval (in ms)
// between each position change. Instead of mapping the motion over a simple 0–100 range,
// the function uses the 401 PWM output steps (from 544 µs to 2500 µs pulse width) to ensure
// smoother transitions and avoid jerky movements.
void ServoOutput::setMovingTime(unsigned int _time){
  movingTime = _time *1000;
  moveTo.intervall = movingTime / 401;
}

void ServoOutput::goTo(int _percentage,bool direct,bool _oPos){
  LOGV("Movement request to %d %, direct movement: %d override position: %d",_percentage,direct,_oPos);
  if(isReferenced() && !direct && movingTime != 0){
    int dutyMicros = map(_percentage, 0, 100, 544, 2500);
    int dutyValue = map(dutyMicros, 0, 20000, 0, 4095); 
    moveTo.destination = _percentage;
    moveTo.destInDuty = dutyValue;
    moveTo.nextStep = readPin();
    moveTo.increment = readPin() < moveTo.destInDuty ? true : false;
    moveTo.actualMillis = millis();
    positioning = true;
    overridePosition = _oPos;

  } else {
    LOGV("Direct Write");
    write(_percentage);
  }
}
/*
bool ServoOutput::goToSlowly(int _angle, bool _overridePosition){

      if(positioning){
         return false;
      }

      int pos = readPosition();
      if(pos < 0 || pos> max){
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
      MoveToSlowly.increment = pos > _angle ? false : true; 
      positioning = true;
      overridePosition = _overridePosition ? true : false;
      return true;
}
*/

void ServoOutput::servoHandler(){

  if(!positioning){
    overridePosition = false;
    return;
  } else {

    if(millis() - moveTo.actualMillis >= moveTo.intervall){
      moveTo.actualMillis=millis();
      moveTo.nextStep += moveTo.increment ? + 1: -1;
      if((moveTo.increment && moveTo.nextStep >= moveTo.destination) ||
        (!moveTo.increment && moveTo.nextStep <= moveTo.destination)){
          ledcWrite(channel, moveTo.destination);
          positioning = false;
        } else {
          ledcWrite(channel, moveTo.nextStep);
        }
    }

  }

  }

void ServoOutput::loop(){
servoHandler();
}

bool ServoOutput::isReferenced(){
  int pos = readPosition();
  if(pos >= 0 && pos<=100){
    return true;
  }
  return false;
}



void ServoOutput::handleMovement(){

  if(isMoving()==false){
    return;
  }

}

#pragma region Configuration


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
  max = 100;
  movingTime = cfg->moveTime * 1000;

}

void ServoOutput::jsonSetup(JsonObject setup){
  LOGI("Servo channel setup");
  channel = -1;
  channel = chMgr->getSlowChannel();
  LOGD("Servo channel assigned at position %d",channel);
  if(channel >= 0){
    pin = setup["pin"].as<unsigned int>();
    min = 0;
    max = 100;
    movingTime = setup["moveTime"].as<unsigned int>() * 1000;
    ledcSetup(channel, 50, 12);
    ledcAttachPin(pin,channel); 
    LOGI("New servo configured, pin: %d, channel: %d, moving time: %d",pin,channel,movingTime);
  } else {
    LOGE("Unable to find a free channel");
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

void ServoOutput::copyJsonCfg(JsonObject incoming,JsonObject retConfig){
    retConfig["pin"] = incoming["pin"].as<unsigned int>();
    retConfig["moveTime"] = incoming["moveTime"].as<unsigned int>();
}



int ServoOutput::validateJsonCfg(JsonObject json){
/*
return code table:
1 validation is ok
-1: pin is not unsigned integer
-10: pin is not asable as output
-8:movingTime not unsigned integer
-10: type is not unsigned integer
-1010: wrong type passed
*/

  if(!json["pin"].is<unsigned int>()){
      return -1;
    } else {
      if(pinUnusable(json["pin"].as<unsigned int>())){
        return -10;
      }
  }
 
  if(!json["moveTime"].is<unsigned int>()){
      return -8;
  }

  return 1;

}

void ServoOutput::getConfiguration(JsonObject cfg){
    cfg["pin"] = pin;
    cfg["moveTime"] = movingTime / 1000;
}

#pragma endregion
