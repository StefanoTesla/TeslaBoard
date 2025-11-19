#include "ServoOutput.h"
#include "esp_log.h"
#include <Arduino.h>
#define LOG_TAG "IOServo"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)
// This library is totally outside the Servo.h arduino library
// I handle the servo like a PWM output and I calcolate the duty in microsecond
// with a 12bit resolution. Since I will use slow timer, I declare that 1° of
// minimum moviment can be fine Going down means fight with float values and I
// don't want to fight, I'm pacific person. I assume olso that minimum position
// is 0° I can't go below zero, or everything here should be rewritte

// this library assume olso that the used servo following the standard servo
// specific 50hz 0°=544us full rotation°=2500us if you need to change it: check
// the write function where 544 and 2500 are the microsecond for 0 and full
// rotation. check the status function: 111 is the ledcRead value when rotation
// is 0° and 511 when servo is fully rotated.

// the Slow Moviment got an auxiliar parameter _overridePosition, if set to
// true, status will return the destination position instead the real angle this
// to bypass a problem with N.I.N.A

int ServoOutput::write(int _pos) {

  if (_pos >= 0 && _pos <= 100) {
    int dutyMicros = map(_pos, 0, 100, 544, 2500);
    int dutyValue = map(dutyMicros, 0, 20000, 0, 4095);
    LOGV("New value requested: %d, dutyValue: %d, channel: %d", _pos, dutyValue,
         channel);
    ledcWrite(channel, dutyValue);
    return 1;
  }
  return 0;
}

int ServoOutput::readPin() { return ledcRead(channel); }

// Return a value from 0 to 100
// can return a negative value if servo was nevere moved
int ServoOutput::readPosition() {
  float angle = ((float)(readPin() - 111) / (511 - 111)) * 100;
  return round(angle);
}

int ServoOutput::status() {
  return overridePosition ? moveTo.destination : readPosition();
}

int ServoOutput::getType() { return 4; }

void ServoOutput::halt() { positioning = false; }

// Sets the servo movement duration and computes the update interval (in ms)
// between each position change. Instead of mapping the motion over a simple
// 0–100 range, the function uses the 401 PWM output steps (from 544 µs to 2500
// µs pulse width) to ensure smoother transitions and avoid jerky movements.


void ServoOutput::goTo(int _percentage, bool direct, bool _oPos) {
  LOGV("Movement request to %d %, direct movement: %d override position: %d",
       _percentage, direct, _oPos);

  if (!isReferenced() || direct || movingTime == 0) {
    LOGV("Direct Write");
    write(_percentage);
    return;
  }
  int dutyMicros = map(_percentage, 0, 100, 544, 2500);
  int dutyValue = map(dutyMicros, 0, 20000, 0, 4095);

  moveTo.startDuty = readPin();  // posizione attuale
  moveTo.destInDuty = dutyValue; // destinazione
  moveTo.startMillis = millis(); // tempo di partenza
  moveTo.totalTime = movingTime; // tempo per tragitto completo
  moveTo.startPercentage = readPosition();
  moveTo.destination = _percentage;

  positioning = true;
  overridePosition = _oPos;
  int distanceInDuty = (moveTo.destInDuty > moveTo.startDuty) ? 
                        (moveTo.destInDuty - moveTo.startDuty) : 
                        (moveTo.startDuty - moveTo.destInDuty);
  moveTo.totalTime = (distanceInDuty * movingTime) / 400;  // Dividi per 400


  LOGV("New movement from %d%% to %d%% (duty %d→%d) performed in: %d ms", moveTo.startPercentage,moveTo.destination, moveTo.startDuty, moveTo.destInDuty,moveTo.totalTime);

}

void ServoOutput::servoHandler() {

  if (!positioning) {
    overridePosition = false;
    return;
  }

  unsigned long elapsed = millis() - moveTo.startMillis;
  if (elapsed >= moveTo.totalTime) {
    LOGV("Positioning completed");
    write(moveTo.destination);
    positioning = false;
    return;
  }

  float t = (float)elapsed / (float)moveTo.totalTime;

  int currentDuty = moveTo.startDuty + (moveTo.destInDuty - moveTo.startDuty) * t;


  ledcWrite(channel, currentDuty);
}

void ServoOutput::loop() { servoHandler(); }

bool ServoOutput::isReferenced() {
  int pos = readPosition();
  LOGV("Actual position: %d", pos);
  if (pos >= 0 && pos <= 100) {
    return true;
  }
  return false;
}

#pragma region Configuration

bool ServoOutput::jsonSetup(JsonObjectConst setup, bool notUsedHere) {
  LOGI("Servo channel setup");
    if(setup["type"].as<int>() != 4){
        return false;
    }
    setName(setup["name"]);
    setDescription(setup["desc"]);
  channel = -1;
  channel = chMgr->getSlowChannel();
  LOGD("Servo channel assigned at position %d", channel);
  if (channel >= 0) {
    pin = setup["pin"].as<unsigned int>();
    min = 0;
    max = 100;
    movingTime = setup["moveTime"].as<unsigned int>() * 1000;
    ledcSetup(channel, 50, 12);
    ledcAttachPin(pin, channel);
    LOGI("New servo configured, pin: %d, channel: %d, moving time: %d", pin,
         channel, movingTime);
  } else {
    LOGE("Unable to find a free channel");
    return false;
  }

  return true;
}

bool ServoOutput::pinUnusable(int pin) {
  if (pin == 1 or pin == 3 or (pin >= 6 and pin <= 11) or pin == 20 or
      pin == 24 or (pin >= 28 and pin <= 31)) {
    return true;
  }

  if (pin > 33) {
    return true;
  }
  return false;
}

void ServoOutput::copyJsonCfg(JsonObject src, JsonObject dest) {
  dest["type"] = 4;
  dest["name"] = src["name"].is<String>() ? src["name"].as<String>() : "";
  dest["desc"] = src["desc"].is<String>() ? src["desc"].as<String>() : "";
  dest["pin"] = src["pin"].as<unsigned int>();
  dest["moveTime"] = src["moveTime"].as<unsigned int>();
}

int ServoOutput::validateJsonCfg(JsonObject json) {
  /*
  return code table:
  1 validation is ok
  -1: pin is not unsigned integer
  -10: pin is not asable as output
  -8:movingTime not unsigned integer
  */

  if (!json["pin"].is<unsigned int>()) {
    return -1;
  } else {
    if (pinUnusable(json["pin"].as<unsigned int>())) {
      return -10;
    }
  }

  if (!json["moveTime"].is<unsigned int>()) {
    return -8;
  }

  return 1;
}

void ServoOutput::getConfiguration(JsonObject cfg) {
  cfg["type"] = 4;
  cfg["name"] = Name;
  cfg["desc"] = Description;
  cfg["pin"] = pin;
  cfg["moveTime"] = movingTime / 1000;
}

void ServoOutput::setMovingTime(unsigned int _time) {
  movingTime = _time * 1000;
}

void ServoOutput::setMoveTime(unsigned int newTime){
  setMovingTime(newTime);
}

#pragma endregion
