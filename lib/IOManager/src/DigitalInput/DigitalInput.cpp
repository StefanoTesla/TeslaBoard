#include <Arduino.h>

#include "DigitalInput.h"

DigitalInput::DigitalInput() {}

bool DigitalInput::jsonSetup(JsonObjectConst obj, bool notUsedHere) {
  if (obj["type"].as<int>() != 1) {
    return false;
  }
  setName(obj["name"]);
  setDescription(obj["desc"]);
  pin = obj["pin"].as<unsigned int>();
  invert = obj["invert"].as<unsigned int>();
  dOn = obj["dOn"].as<unsigned int>();
  dOff = obj["dOff"].as<unsigned int>();
  min = 0;
  max = 1;
  pinMode(pin, INPUT);
  return true;
}

//
// return true if you can't use this pin
bool DigitalInput::pinUnusable(int pin) {
  if (pin == 1 or pin == 3 or (pin >= 6 and pin <= 11) or pin == 20 or
      pin == 24 or (pin >= 28 and pin <= 31) or pin == 37 or pin == 38 or
      pin > 39) {
    return true;
  }

  if (pin == 0 or pin == 12) {
    return true;
  }
  return false;
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

void DigitalInput::copyJsonCfg(JsonObject src, JsonObject dest) {
  dest["type"] = 1;
  dest["name"] = src["name"].is<String>() ? src["name"].as<String>() : "";
  dest["desc"] = src["desc"].is<String>() ? src["desc"].as<String>() : "";
  dest["pin"] = src["pin"];
  dest["invert"] = src["invert"];
  dest["dOn"] = src["dOn"];
  dest["dOff"] = src["dOff"];
}

void DigitalInput::getConfiguration(JsonObject cfg) {
  cfg["type"] = 1;
  cfg["name"] = Name;
  cfg["desc"] = Description;
  cfg["pin"] = pin;
  cfg["invert"] = invert;
  cfg["dOn"] = dOn;
  cfg["dOff"] = dOff;
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
  /* ON CASE */
  if (readPin()) {

    if (!value) {
      if (!reInput) { /* get rising edge*/
        feInput = false;
        reInput = true;
        ackMillis = millis();
      } else {
        if (millis() - ackMillis > dOn) { /*wait ton time*/
          value = 1;
        }
      }
    }
  } else {
    if (value) {
      if (!feInput) { /* get falling edge*/
        reInput = false;
        feInput = true;
        ackMillis = millis();
      } else {
        if (millis() - ackMillis > dOff) { /*wait toff time*/
          value = 0;
        }
      }
    }
  }

  return value;
}

void DigitalInput::loop() { status(); }

int DigitalInput::getType() { return 1; }
