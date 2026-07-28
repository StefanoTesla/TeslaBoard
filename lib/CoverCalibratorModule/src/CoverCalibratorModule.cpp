#include "CoverCalibratorModule.h"
#include "esp_log.h"
#define LOG_TAG "CovCal"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

#pragma region Configuration
/* here we write additional data if nvs was empty*/
void CoverCalibratorModule::initSecondaryData() {
    NvsManager::getInstance().putString("calibrator", "{}");
    NvsManager::getInstance().putString("cover", "{}");
}

/* here we load secondary data during the begin */
void CoverCalibratorModule::loadSecondaryData() {
    String cfg;
    tmpCfg.clear();

    /* load calibrator data */
    LOGI("deserialization of calibrator json configuration");
    cfg = NvsManager::getInstance().getString("calibrator","{}");
    LOGD("raw calibrator json is: %s",cfg.c_str());
    DeserializationError error = deserializeJson(tmpCfg, cfg);
    LOGD("calibrator deserialization ret val: %d 0=no error",error);
    if(!error){
        calibrator.begin(tmpCfg);
    }

    /* load cover data */
    cfg = NvsManager::getInstance().getString("cover","{}");
    LOGD("raw cover json is: %s",cfg.c_str());
    error = deserializeJson(tmpCfg, cfg);
    LOGD("cover cover ret val: %d 0=no error",error);

    if(!error){
        cover.begin(tmpCfg);
    }
    tmpCfg.clear();
}


/* here we update the nvs when new schema is given */
bool CoverCalibratorModule::applySchemaUpgradeStep(uint16_t currentVersion) {
    LOGI("Applying schema upgrade step from version %u", currentVersion);

    if (!NvsManager::getInstance().openNVS(false, COVERC_SCHEMA_NAME)) {
        LOGE("Unable to open board namespace for schema upgrade");
        return false;
    }

    switch (currentVersion) {
        case 0:
            NvsManager::getInstance().putString("identifier", "CoverC");
            NvsManager::getInstance().putInt("schema", 1);
            NvsManager::getInstance().closeNVS();
            return true;

        default:
            LOGE("Unknown schema version %u for board upgrade", currentVersion);
            NvsManager::getInstance().closeNVS();
            return false;
    }
}

/* here we read secondary data during the get config */
void CoverCalibratorModule::appendSecondaryConfig(JsonObject dest) {
    JsonObject calibObj = dest["calibrator"].to<JsonObject>();
    calibrator.getConfiguration(calibObj);
    JsonObject coverObj = dest["cover"].to<JsonObject>();
    cover.getConfiguration(coverObj);
}


/* here the validation of secondary data when store configuration is called*/
bool CoverCalibratorModule::validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) {
    JsonArray err = response["errors"].as<JsonArray>();

    if(!toBeValidated["calibrator"].is<JsonObject>()){
        err.add("Calibrator Configuration is missing");
        LOGE("Calibrator object is missing");
        return false;
    }

    calibrator.validateConfiguration(toBeValidated["calibrator"],response);

    if(err.size()>0){
        return false;
    }

    if(!toBeValidated["cover"].is<JsonObject>()){
        LOGE("Cover object is missing");
        err.add("Calibrator Configuration is missing");
        return false;
    }

    cover.validateConfiguration(toBeValidated["cover"],response);

    if(err.size()>0){
        return false;
    }

    rebootNeeded = response["reboot"].as<bool>();

    return err.size() == 0;
}

/* here we store secondary data during the save config */
void CoverCalibratorModule::storeSecondaryConfig(const JsonObject &toBeStored) {
    LOGI("Writing calibrator config");
    calibrator.storeConfiguration(toBeStored["calibrator"]);

    LOGI("Writing cover config");
    cover.storeConfiguration(toBeStored["cover"]);
}


#pragma endregion


void CoverCalibratorModule::loop(){
    
    if(isEnable()){
        if(calibrator.isEnable()){
            calibrator.loop();
        }
        if(cover.isEnable()){
            cover.loop();
        }
    }
}



/* SERIAL MANAGER */

bool CoverCalibratorModule::handlePacket(char* payload, Stream& out) {
    char* saveptr = nullptr;
    char* cmd = strtok_r(payload, ":", &saveptr);

    if (cmd == nullptr) {
      out.print("<ERR:SWITCH:BAD_CMD>");
      return false;
    }

    if (strcmp(cmd, "SET") == 0) {
      char* chStr = strtok_r(nullptr, ":", &saveptr);
      char* valueStr = strtok_r(nullptr, ":", &saveptr);

      if (chStr == nullptr || valueStr == nullptr) {
        out.print("<ERR:SWITCH:BAD_PARAM>");
        return false;
      }

      const int channel = atoi(chStr);
      const int value = atoi(valueStr);

      if (channel < 0 || channel > 16) {
        out.print("<ERR:SWITCH:RANGE>");
        return false;
      }

      if (value < 0 || value > 4096) {
        out.print("<ERR:SWITCH:RANGE>");
        return false;
      }

      // TODO: setSwitch(channel, value);
      out.print("<OK:SWITCH:SET>");
      return true;
    }

    if (strcmp(cmd, "GET") == 0) {
      char* chStr = strtok_r(nullptr, ":", &saveptr);

      if (chStr == nullptr) {
        out.print("<ERR:SWITCH:BAD_PARAM>");
        return false;
      }

      const int channel = atoi(chStr);

      if (channel < 0 || channel > 16) {
        out.print("<ERR:SWITCH:RANGE>");
        return false;
      }

      // TODO: int value = getSwitch(channel);
      const int value = 1234;

      out.print("<OK:SWITCH:GET:");
      out.print(channel);
      out.print(":");
      out.print(value);
      out.print(">");
      return true;
    }

    if (strcmp(cmd, "PULSE") == 0) {
      char* chStr = strtok_r(nullptr, ":", &saveptr);
      char* msStr = strtok_r(nullptr, ":", &saveptr);

      if (chStr == nullptr || msStr == nullptr) {
        out.print("<ERR:SWITCH:BAD_PARAM>");
        return false;
      }

      const int channel = atoi(chStr);
      const int durationMs = atoi(msStr);

      if (channel < 0 || channel > 16) {
        out.print("<ERR:SWITCH:RANGE>");
        return false;
      }

      if (durationMs <= 0 || durationMs > 60000) {
        out.print("<ERR:SWITCH:RANGE>");
        return false;
      }

      // TODO: startPulse(channel, durationMs);
      out.print("<OK:SWITCH:PULSE>");
      return true;
    }

    if (strcmp(cmd, "STATUS") == 0) {
      out.print("<OK:SWITCH:READY>");
      return true;
    }

    out.print("<ERR:SWITCH:UNKNOWN_CMD>");
    return false;
  }
