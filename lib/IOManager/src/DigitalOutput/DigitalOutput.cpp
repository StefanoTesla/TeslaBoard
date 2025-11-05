#include "DigitalOutput.h"
#include <Arduino.h>

DigitalOutput::DigitalOutput() {}

bool DigitalOutput::jsonSetup(JsonObjectConst obj, bool notUsedHere) {
  if (obj["type"].as<int>() != 2) {
    Serial.println("TYPE ERROR");
    return false;
  }
  setName(obj["name"]);
  setDescription(obj["desc"]);
  pin = obj["pin"].as<unsigned int>();
  invert = obj["invert"].as<unsigned int>();
  pinMode(pin, OUTPUT);
  Serial.print("New DO setup at pin: ");
  Serial.println(pin);
  min = 0;
  max = 1;
  write(0);
  return true;
}

bool DigitalOutput::pinUnusable(int pin) {
  if (pin == 1 or pin == 3 or (pin >= 6 and pin <= 11) or pin == 20 or
      pin == 24 or (pin >= 28 and pin <= 31)) {
    return true;
  }

  if (pin > 33) {
    return true;
  }
  return false;
}

int DigitalOutput::validateJsonCfg(JsonObject json) {

  if (!json["pin"].is<unsigned int>()) {
    return -1;
  } else {
    if (pinUnusable(json["pin"].as<unsigned int>())) {
      return -10;
    }
  }
  if (!json["invert"].is<bool>()) {
    return -4;
  }

  return 1;
}

/**
 * @brief This funcition copy only the essential json key.
 *
 * This function copy the json configuration keys requested by the DigitalOutput
 * This function copy the src["pin"] and src["invert"] to the dest object
 *
 * @param src where key are present
 * @param dest where keys are copied
 *
 * @return nothing
 */
void DigitalOutput::copyJsonCfg(JsonObject src, JsonObject dest) {
  dest["type"] = 2;
  dest["name"] = src["name"].is<String>() ? src["name"].as<String>() : "";
  dest["desc"] = src["desc"].is<String>() ? src["desc"].as<String>() : "";
  dest["pin"] = src["pin"];
  dest["invert"] = src["invert"];
}

void DigitalOutput::getConfiguration(JsonObject cfg) {
  cfg["type"] = 2;
  cfg["name"] = Name;
  cfg["desc"] = Description;
  cfg["pin"] = pin;
  cfg["invert"] = invert;
}

/**
 * @brief Write the value to the gpio.
 *
 * Questa funzione utilizza `digitaldigitalWrite()` per scrivere il valore
 * di un pin, se configurato, lo inverte
 *
 * @param _value value to be written to the GPIO
 * @return int 1= operazione completata.
 */
int DigitalOutput::write(int _value) {
  // never trust what user can write in _value
  if (invert) {
    if (_value == 0) {
      _value = 1;
    } else {
      _value = 0;
    }
  }
  digitalWrite(pin, _value);
  return 1;
}

void DigitalOutput::setInvert(bool newInvert) { invert = newInvert; }

int DigitalOutput::readPin() { return digitalRead(pin); }

int DigitalOutput::status() {

  value = invert ? !readPin() : readPin();
  return value;
}

int DigitalOutput::getType() { return 2; }
