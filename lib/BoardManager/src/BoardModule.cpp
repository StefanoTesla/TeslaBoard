#include "BoardModule.h"
#include "esp_log.h"

#undef LOG_TAG
#define LOG_TAG "Board"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

void BoardModule::initSecondaryData() {
    NvsManager::getInstance().putString("locale", "en");
}

void BoardModule::loadSecondaryData() {
    locale = NvsManager::getInstance().getString("locale", "en");
    LOGV("Board identifier: %s", identifier.c_str());
    LOGV("Board locale: %s", locale.c_str());
}

void BoardModule::appendSecondaryConfig(JsonObject dest) {
    dest["locale"] = locale;
}

bool BoardModule::validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) {
    JsonArray err = response["errors"].as<JsonArray>();

    if (!toBeValidated["locale"].is<const char*>()) {
        err.add("Locale is not a string");
        LOGE("Locale is not a string");
        return false;
    }

    String tmp = toBeValidated["locale"].as<String>();
    if (tmp != "en" && tmp != "it" && tmp != "de" && tmp != "fr" && tmp != "es") {
        err.add("Locale doesn't exist");
        LOGE("Locale doesn't exist");
        return false;
    }

    return true;
}

void BoardModule::storeSecondaryConfig(const JsonObject &toBeStored) {
    locale = toBeStored["locale"].as<String>();
    NvsManager::getInstance().putString("locale", locale);
}

bool BoardModule::applySchemaUpgradeStep(uint16_t currentVersion) {
    LOGI("Applying schema upgrade step from version %u", currentVersion);

    if (!NvsManager::getInstance().openNVS(false, BOARD_SCHEMA_NAME)) {
        LOGE("Unable to open board namespace for schema upgrade");
        return false;
    }

    switch (currentVersion) {
        case 0:
            NvsManager::getInstance().putString("identifier", "TeslaBoard");
            NvsManager::getInstance().putString("locale", "en");
            NvsManager::getInstance().putInt("schema", 1);
            NvsManager::getInstance().closeNVS();
            return true;

        case 1:
            NvsManager::getInstance().putBool("enable", true);
            NvsManager::getInstance().putInt("order", 1);
            NvsManager::getInstance().putInt("schema", 2);
            NvsManager::getInstance().closeNVS();
            return true;


        default:
            LOGE("Unknown schema version %u for board upgrade", currentVersion);
            NvsManager::getInstance().closeNVS();
            return false;
    }
}

void BoardModule::loop() {
    if (millis() - ackMillis >= 60000) {
        ackMillis = millis();
        upTime++;
    }
}




/* SERIAL MANAGER */

bool BoardModule::handlePacket(char* payload, Stream& out) {
    char* saveptr = nullptr;
    char* cmd = strtok_r(payload, ":", &saveptr);

    if (cmd == nullptr) {
      out.print("<ERR:SWITCH:BAD_CMD>");
      return false;
    }

    if (strcmp(cmd, "READ") == 0) {
        char* nameSpace = strtok_r(nullptr, ":", &saveptr);
        char* type = strtok_r(nullptr, ":", &saveptr);
        char* variable = strtok_r(nullptr, ":", &saveptr);
        LOGI("richiesta di lettura della nvs");
        if(!NvsManager::getInstance().openNVS(true,nameSpace)){
            out.print("<ERR:BO:NVS_NO_OPEN>");
            closeNVS();
            return false;
        }
        if (strcmp(type, "BOOL") == 0 ){
            out.print(NvsManager::getInstance().getBool(variable));
        } else if (strcmp(type, "INT") == 0 ){
            out.print(NvsManager::getInstance().getInt(variable));
        } 
        else if (strcmp(type, "STR") == 0 ){
            out.print(NvsManager::getInstance().getString(variable));
        } else {
             out.print("<ERR:BO:WRONG_TYPE>");
        }
        
        closeNVS();
        return true;


    }

    out.print("<ERR:SWITCH:UNKNOWN_CMD>");
    return false;
  }
