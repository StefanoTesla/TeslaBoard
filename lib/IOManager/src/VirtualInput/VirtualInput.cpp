#include "VirtualInput.h"
#include <Arduino.h>
#include "esp_log.h"
#undef LOG_TAG
#define LOG_TAG "VirtualIn"
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

VirtualInput::VirtualInput() {}

bool VirtualInput::jsonSetup(JsonObjectConst obj, bool notUsedHere) {
  if (obj["type"].as<int>() != 5) {
    return false;
  }

  setupCommonJson(obj);

  min = INT_MIN;
  max = INT_MAX;
  expiration = obj["expiration"].as<int>() * 1000UL;
  defaultVal = obj["defaultValue"].as<int>();
  value = defaultVal;
  lastRefresh = 0;
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
  copyCommonJsonCfg(src, dest);
  dest["type"] = 5;
  dest["defaultValue"] = src["defaultValue"];
  dest["expiration"] = src["expiration"];
}

void VirtualInput::getConfiguration(JsonObject cfg) {
  cfg["type"] = 5;
  getCommonConfiguration(cfg);
  cfg["defaultValue"] = defaultVal;
  cfg["expiration"] = expiration / 1000UL;
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
  lastRefresh = millis();
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
  expiration = _newExpiration * 1000UL;
}

void VirtualInput::loop() {
}


bool VirtualInput::isExpired() {
  if (expiration == 0)  return false;
  if (lastRefresh == 0) return true;         
  return (millis() - lastRefresh) > expiration;
}