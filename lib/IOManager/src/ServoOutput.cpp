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

ServoOutput::ServoOutput(){
    
}

void ServoOutput::setup(IOConfigBase* config){
  if (config->getType() == 5) { 
    ServoutputConfig* cfg = static_cast<ServoutputConfig*>(config);
    pin = cfg->pin;
    channel = cfg->channel;
    ledcAttachPin(pin, channel);
    Serial.print("New Servo setup at pin: ");
    Serial.print(pin);
    Serial.print(" at channel: ");
    Serial.println(channel);
} else {
    Serial.println("Errore: SERVO tipo di configurazione non valido!");
}
}

int ServoOutput::write(int _angle) {
    if (_angle > 0 && _angle <= maxDeg){
      int dutyMicros = map(_angle, 0, maxDeg, 544, 2500);
      int dutyValue = map(dutyMicros, 0, 20000, 0, 4095); 
      ledcWrite(channel, dutyValue);
      return 1;
    }

    return 0;

}

int ServoOutput::readPin() {
    return ledcRead(channel);
}

int ServoOutput::status(){
    if(overridePosition){
      return MoveToSlowly.destination;
    } else {
      float angle = ((float)(readPin() - 111) / (511 - 111)) * maxDeg;
      return round(angle);
    }

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



bool ServoOutput::goToSlowly(int _angle, bool _overridePosition){

      if(moving){
        Serial.println("Servo is already moving");
         return false;
      }

      if(status() < 0 || status()> maxDeg){
        Serial.println("Servo is in an undefined position, use write function");
        return false;
      }

      if(_angle < 0 || _angle > maxDeg){
        Serial.println("invalid commanded angle.");
        return false;
      }

      // get in how many ms I need to do a degree
      MoveToSlowly.destination = _angle;
      MoveToSlowly.intervall = movingTime / maxDeg;
      if(MoveToSlowly.intervall == 0){
        MoveToSlowly.intervall = 1;
      }
      MoveToSlowly.increment = status() > _angle ? false : true; 
      moving = true;
      overridePosition = _overridePosition ? true : false;
      return true;
}


void ServoOutput::servoHandler(){
switch (cycle){
    case 0:
      MoveToSlowly.actualMillis = millis();
      MoveToSlowly.nextDeg = status();
      cycle =10;
      break;
    case 10:
      if(millis() - MoveToSlowly.actualMillis > MoveToSlowly.intervall){
        MoveToSlowly.nextDeg += MoveToSlowly.nextDeg ? -1 : +1;
        if((MoveToSlowly.increment && MoveToSlowly.nextDeg > MoveToSlowly.destination) ||
            (!MoveToSlowly.increment && MoveToSlowly.nextDeg < MoveToSlowly.destination)) {
              write(MoveToSlowly.nextDeg);
              MoveToSlowly.actualMillis = millis();
        } else {
          write(MoveToSlowly.destination);
          cycle = 0;
          moving = false;
        }
      }

  }
}


void ServoOutput::loop(){
  if(moving){
    servoHandler();
  }
}