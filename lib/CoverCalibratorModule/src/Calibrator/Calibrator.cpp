#include "Calibrator.h"
#include "esp_log.h"
#define LOG_TAG "Calib"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)
/* Setup the calibrator */
void Calibrator::begin(JsonDocument doc) {

  Serial.println("Calibrator begin");

  moduleEnable = doc["enable"];

  if (moduleEnable) {

    JsonObject calib = doc["outPWM"];
    calib["name"] = "Calibrator";
    calibrator.jsonSetup(calib, true);
  }
}

/* loop cycle, status and cycle update */
void Calibrator::loop() { updateStatus(); }

void Calibrator::updateStatus() {
  if (moduleEnable) {
    if (getBrightness() == 0) {
      status = Off;
    } else {
      status = Ready;
    }
  } else {
    LOGV("module is gone!");
    status = NotPresent;
  }
}

Calibrator::Status Calibrator::getStatus() const { return status; }

/*
Configuration Area
*/

void Calibrator::getConfiguration(JsonObject obj) {

  obj["enable"] = moduleEnable;

  JsonObject outPWM = obj["outPWM"].to<JsonObject>();
  calibrator.getConfiguration(outPWM);
}

void Calibrator::validateConfiguration(const JsonObject &obj,
                                       JsonObject response) {

  JsonArray err = response["errors"].to<JsonArray>();
  int retVal = 0;

  if (!obj["outPWM"].is<JsonObject>()) {
    err.add("calibrator is missing");
    return;
  }

  JsonObject calibPin = obj["outPWM"];
  retVal = calibrator.validateJsonCfg(calibPin);

  if (retVal != 1) {
    JsonObject e = err.add<JsonObject>();
    e["id"] = 1;
    e["error"] = retVal;
    return;
  }

  /* check if board need a reboot */

  if (calibPin["pin"].as<unsigned int>() != calibrator.getPinNumber()) {
    response["reboot"] = true;
  }
}

void Calibrator::storeConfiguration(JsonObject calibratorObject,
                                    const char *schema) {

  tmpCfg.clear();

  tmpCfg["enable"] = calibratorObject["enable"];

  if (tmpCfg["enable"].as<bool>()) {
    JsonObject outPWM = tmpCfg["outPWM"].to<JsonObject>();
    calibrator.copyJsonCfg(calibratorObject["outPWM"], outPWM);
  }

  String json;
  serializeJson(tmpCfg, json);

  Preferences pref;
  pref.begin(schema);

  pref.putString("calibrator", json);
  pref.end();
  tmpCfg.clear();
}