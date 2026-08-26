#include "VirtualInput.h"
#include <Arduino.h>

VirtualInput::VirtualInput() {}

bool VirtualInput::jsonSetup(JsonObjectConst obj, bool notUsedHere) {
  if (obj["type"].as<int>() != 4) {
    return false;
  }

  setupCommonJson(obj);

  min = INT_MIN;
  max = INT_MAX;
  write(0);
  return true;
}

bool VirtualInput::pinUnusable(int pin) {
  return false;
}

int VirtualInput::validateJsonCfg(JsonObject json) {

  if (!json["default"].is<int32_t>()) {
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
  dest["type"] = 4;
  copyCommonJsonCfg(src, dest);

}

void VirtualInput::getConfiguration(JsonObject cfg) {
  cfg["type"] = 4;
  getCommonConfiguration(cfg);
}

/**
 * @brief Write the value to the gpio.
 *
 * Questa funzione utilizza `digitalWrite()` per scrivere il valore
 * di un pin, se configurato, lo inverte
 *
 * @param _value value to be written to the GPIO
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

int VirtualInput::getType() { return 4; }
