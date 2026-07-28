#include "DomeModule.h"
#undef LOG_TAG
#define LOG_TAG "Dome"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

#pragma region Configuration
/* here we write additional data if nvs was empty*/
void DomeModule::initSecondaryData() {
    NvsManager::getInstance().putString("shutter", "{}");
}

/* here we load secondary data during the begin */
void DomeModule::loadSecondaryData() {

    String cfg = NvsManager::getInstance().getString("shutter","{}");
    LOGD("raw shutter json is: %s",cfg.c_str());
    DeserializationError error = deserializeJson(tmpCfg, cfg);
    
    LOGD("shutter deserialization ret val: %d 0=no error",error);

    if(!error){
        shutter.begin(tmpCfg);
    }

    tmpCfg.clear();
}


/* here we update the nvs when new schema is given */
bool DomeModule::applySchemaUpgradeStep(uint16_t currentVersion) {
    LOGI("Applying schema upgrade step from version %u", currentVersion);

    if (!NvsManager::getInstance().openNVS(false, DOME_SCHEMA_NAME)) {
        LOGE("Unable to open board namespace for schema upgrade");
        return false;
    }

    switch (currentVersion) {
        case 0:
            NvsManager::getInstance().putString("identifier", "Dome");
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
void DomeModule::appendSecondaryConfig(JsonObject dest) {
    JsonObject shutterObj = dest["shutter"].to<JsonObject>();
    shutter.getConfiguration(shutterObj);
}


/* here the validation of secondary data when store configuration is called*/
bool DomeModule::validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) {
    JsonArray err = response["errors"].as<JsonArray>();
    if(!toBeValidated["shutter"].is<JsonObject>()){
        LOGE("Shutter Configuration is missing or is not a json object");
        err.add("Shutter Configuration is missing");
        return false;
    }
    
    shutter.validateConfiguration(toBeValidated["shutter"],response);
    return err.size() == 0;
}

/* here we store secondary data during the save config */
void DomeModule::storeSecondaryConfig(const JsonObject &toBeStored) {
    shutter.storeConfiguration(toBeStored["shutter"],DOME_SCHEMA_NAME);
}



#pragma endregion


bool DomeModule::isEnable(){
    return moduleEnable;
}


void DomeModule::loop(){
    
    if(isEnable()){
        shutter.loop();
    }
}



/* SERIAL MANAGER */

bool DomeModule::handlePacket(char* payload, Stream& out) {
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
