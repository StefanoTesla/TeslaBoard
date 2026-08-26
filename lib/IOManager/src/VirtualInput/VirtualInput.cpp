#include "VirtualInput.h"
#include <Arduino.h>

VirtualInput::VirtualInput() {}

bool VirtualInput::jsonSetup(JsonObjectConst obj, bool notUsedHere) {
  if (obj["type"].as<int>() != 5) {
    return false;
  }

  setupCommonJson(obj);

  min = INT_MIN;
  max = INT_MAX;
  expiration = obj["exiration"].as<int>();
  write(obj["defaultValue"].as<int>());
  return true;
}

bool VirtualInput::pinUnusable(int pin) {
  return false;
}

int VirtualInput::validateJsonCfg(JsonObject json) {

  if (!json["defaultValue"].is<int32_t>()) {
    return -1;
  }

  if (!json["expiration"].is<int32_t>()) {
    return -1;
  }
  if (json["expiration"].as<int32_t>() < 0) {
    return -1;
  }

  return 1;
}

/**
 * @brief This funcition copy only the essential json key.
 *
 * This function copy the json configuration keys requested by the VirtualInput
 * This function copy the src["pin"] and src["invert"] to the dest object
 *
 * @param src where key are present
 * @param dest where keys are copied
 *
 * @return nothing
 */
void VirtualInput::copyJsonCfg(JsonObject src, JsonObject dest) {
  dest["type"] = 5;
  copyCommonJsonCfg(src, dest);
  dest["defaultValue"] = src["defaultValue"];
  dest["expiration"] = src["expiration"];

}

void VirtualInput::getConfiguration(JsonObject cfg) {
  cfg["type"] = 5;
  getCommonConfiguration(cfg);
  cfg["defaultValue"] = defaultVal;
  cfg["expiration"] = expiration;
}

/**
 * @brief Write the value to the gpio.
 *
 * Questa funzione utilizza `digitalWrite()` per scrivere il valore
 * di un pin, se configurato, lo inverte
 *
 * @param _value value to be written to the Virtual Input
 * @return int 1= operazione completata.
 */
int VirtualInput::write(int32_t _value) {
  value = _value;
  return 1;
}

int VirtualInput::readPin() { return value; }

int VirtualInput::status() {
  return value;
}

int VirtualInput::getType() { return 5; }


void VirtualInput::setDefault(int _newDefaul) {
  defaultVal = _newDefaul;
}
void VirtualInput::setExpiration(int _newExpiration) {
  expiration = _newExpiration * 100;
}

void VirtualInput::loop() {
}