#include "Calibrator.h"
#include "esp_log.h"
#define LOG_TAG "Calib"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)


#pragma region nvsHandler

bool Calibrator::openNVS(bool readOnly) {

  switch (nvsStatus) {

    // nvs is closed, i need to open according by the readOnly
  case CLOSED:
    LOGV("NVS seems to be closed");
    if (nvs.begin(COVERC_SCHEMA_NAME, readOnly)) {
      if (readOnly) {
        LOGV("NVS opened in readonly");
        nvsStatus = OPEN_READOLNY;
      } else {
        LOGV("NVS opened with write rights");
        nvsStatus = OPEN_WRITE;
      }
      return true;
    } else {
      LOGE("Error opening the NVS");
      nvsStatus = CLOSED;
      return false;
    }
    break;

  case OPEN_READOLNY:
    LOGV("NVS seems open in read only");
    if (!readOnly) {
      closeNVS();
      if (nvs.begin(COVERC_SCHEMA_NAME, false)) {
        nvsStatus = OPEN_WRITE;
        LOGV("NVS opened with write rights");
        return true;
      } else {
        LOGV("Error during opening NVS with write rights");
        return false;
      }
    } else {
      LOGV("NVS already open in read only");
      return true;
    }
    break;

  case OPEN_WRITE:
    LOGV("NVS seems open with write rights");
    if (readOnly) {
      closeNVS();
      nvsStatus = CLOSED;
      if (nvs.begin(COVERC_SCHEMA_NAME, true)) {
        nvsStatus = OPEN_READOLNY;
        LOGV("NVS opened in read only");
        return true;
      } else {
        LOGV("Error during opening NVS in read only");
        return false;
      }
    } else {
      LOGV("NVS already open with write rights");
      return true;
    }
    break;

  default:
    LOGE("Unknown NVS status: %d", nvsStatus);
    return false;
    break;
  }

  LOGE("Arrived at the buttom of the function, don't know what happed..");
  return false;
}

void Calibrator::closeNVS() {
  if (nvsStatus != CLOSED) {
    nvs.end();
    nvsStatus = CLOSED;
    LOGV("NVS closed");
  } else {
    LOGV("NVS already closed");
  }
}

#pragma endregion
/* Setup the calibrator */
void Calibrator::begin(const JsonDocument& doc) {

  LOGV("Calibrator begin");

  moduleEnable = doc["enable"];

  if (moduleEnable) {

    JsonObjectConst calib = doc["outPWM"];
    calibrator.jsonSetup(calib, true);
  }

  LOGV("Calibrator END");
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

  if (!obj["enable"].is<bool>()) {
    err.add("enable is missing");
    return;
  }

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

  if (calibPin["pin"].as<unsigned int>() != calibrator.getPinNumber()
      || moduleEnable != obj["enable"].as<bool>()) {
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

  openNVS(false);

  nvs.putString("calibrator", json);
  closeNVS();
  tmpCfg.clear();
}