#include "Calibrator.h"
#include "esp_log.h"
#define LOG_TAG "Calib"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)


#pragma region Configuration

void Calibrator::begin(const JsonDocument& doc) {

  LOGV("Calibrator begin");

  moduleEnable = doc["enable"];

  if (moduleEnable) {

    JsonObjectConst calib = doc["outPWM"];
    calibrator.jsonSetup(calib, true);
  }

  LOGV("Calibrator END");
}

void Calibrator::getConfiguration(JsonObject obj) {

  obj["enable"] = moduleEnable;

  JsonObject outPWM = obj["outPWM"].to<JsonObject>();
  calibrator.getConfiguration(outPWM);
}

void Calibrator::validateConfiguration(const JsonObject &obj, JsonObject response) {

  JsonArray err = response["errors"].to<JsonArray>();
  int retVal = 0;

  if (!obj["enable"].is<bool>()) {
    err.add("enable is missing");
    return;
  }

  if (!obj["outPWM"].is<JsonObject>()) {
    err.add("calibrator is missing");
    return;
  }

  bool incomingEnable = obj["enable"].as<bool>();

  //module enable is changed from new data, reboot is neeed of course
  if (moduleEnable != incomingEnable) {
    response["reboot"] = true;
  }

  // the new configuration disable the calibrator, nothing to do more
  if(!incomingEnable){
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

  /* Access to the pin number only if the module is enable, otherwise nullpointer exception */
  if(moduleEnable && calibPin["pin"].as<unsigned int>() != calibrator.getPinNumber()){
      response["reboot"] = true;
  }

}

void Calibrator::storeConfiguration(JsonObject calibratorObject) {

  tmpCfg.clear();

  tmpCfg["enable"] = calibratorObject["enable"];

  if (tmpCfg["enable"].as<bool>()) {
    JsonObject outPWM = tmpCfg["outPWM"].to<JsonObject>();
    calibrator.copyJsonCfg(calibratorObject["outPWM"], outPWM);
  }

  String json;
  serializeJson(tmpCfg, json);

  NvsManager::getInstance().putString("calibrator", json);
  tmpCfg.clear();
}

#pragma endregion


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
    status = NotPresent;
  }
}

Calibrator::Status Calibrator::getStatus() const { return status; }



