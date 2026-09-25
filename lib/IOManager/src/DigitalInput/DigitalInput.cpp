#include <Arduino.h>

#include "DigitalInput.h"
#include "esp_log.h"
#undef LOG_TAG
#define LOG_TAG "DIn"
#ifdef IOBASE_LOG
  #define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
  #define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
  #define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
  #define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
  #define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)
#else
  #define LOGV(...) do {} while (0)
  #define LOGD(...) do {} while (0)
  #define LOGI(...) do {} while (0)
  #define LOGW(...) do {} while (0)
  #define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)
#endif

DigitalInput::DigitalInput() {}

bool DigitalInput::jsonSetup(JsonObjectConst obj, bool notUsedHere) {
  if (obj["type"].as<int>() != 1) {
    return false;
  }

  setupCommonJson(obj);

  pin = obj["pin"].as<unsigned int>();
  invert = obj["invert"].as<unsigned int>();
  dOn = obj["dOn"].as<unsigned int>();
  dOff = obj["dOff"].as<unsigned int>();

  min = 0;
  max = 1;

  pinMode(pin, INPUT);
  return true;
}

void DigitalInput::copyJsonCfg(JsonObject src, JsonObject dest) {
  dest["type"] = 1;
  copyCommonJsonCfg(src, dest);
  dest["pin"] = src["pin"];
  dest["invert"] = src["invert"];
  dest["dOn"] = src["dOn"];
  dest["dOff"] = src["dOff"];
}

void DigitalInput::getConfiguration(JsonObject cfg) {
  cfg["type"] = 1;
  getCommonConfiguration(cfg);
  cfg["pin"] = pin;
  cfg["invert"] = invert;
  cfg["dOn"] = dOn;
  cfg["dOff"] = dOff;
}


int DigitalInput::validateJsonCfg(JsonObject json) {

  serializeJson(json, Serial);
  if (!json["pin"].is<unsigned int>()) {
    return -1;
  } else {
    if (pinUnusable(json["pin"].as<unsigned int>())) {
      return -10;
    }
  }
  if (!json["dOn"].is<unsigned int>()) {
    return -2;
  }
  if (!json["dOff"].is<unsigned int>()) {
    return -3;
  }
  if (!json["invert"].is<bool>()) {
    return -4;
  }

  return 1;
}
//
// return true if you can't use this pin
bool DigitalInput::pinUnusable(int pin) {

  #ifdef IS_ESP32S3
  if(pin <=0 or pin == 3 or pin == 19 or pin == 20 or (pin >=22 and pin <= 38) or (pin >=43 and pin <= 47) or pin >= 48){
    return true;
  } 

  return false;

  #else

  if (pin < 1 or pin == 3 or (pin >= 6 and pin <= 12) or pin == 20 or
      pin == 24 or (pin >= 28 and pin <= 31) or pin == 37 or pin == 38 or
      pin > 39) {
    return true;
  }

  return false;

  #endif
}



void DigitalInput::setDelays(unsigned int newDOn, unsigned int newDOff) {
  dOn = newDOn;
  dOff = newDOff;
}
void DigitalInput::setInvert(bool newInvert) { invert = newInvert; }

int DigitalInput::write(int _value) {
  return -1;
}

int DigitalInput::readPin() {
  return invert ? !digitalRead(pin) : digitalRead(pin);
}

int DigitalInput::status() {
  bool raw = readPin();

  if (raw == value) {
    reInput = false;
    feInput = false;
    return value;
  }

  if (!reInput && !feInput) {
    ackMillis = millis();
    reInput = (raw == true);
    feInput = (raw == false);
  }

  unsigned long threshold = raw ? dOn : dOff;
  if (millis() - ackMillis > threshold) {
    value = raw ? 1 : 0;
    reInput = false;
    feInput = false;
  }

  return value;
}

void DigitalInput::loop() { status(); }

int DigitalInput::getType() { return 1; }
