#include "BoardModule.h"
#include "esp_log.h"
#define LOG_TAG "Board"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

#pragma region nvsHandler

bool BoardModule::openNVS(bool readOnly) {

  switch (nvsStatus) {

    // nvs is closed, i need to ope according by the readOnly
  case CLOSED:
    LOGV("NVS seems to be closed");
    if (nvs.begin(BOARD_SCHEMA_NAME, readOnly)) {
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
      if (nvs.begin(BOARD_SCHEMA_NAME, false)) {
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
      if (nvs.begin(BOARD_SCHEMA_NAME, true)) {
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

void BoardModule::closeNVS() {
  if (nvsStatus != CLOSED) {
    nvs.end();
    nvsStatus = CLOSED;
    LOGV("NVS closed");
  } else {
    LOGV("NVS already closed");
  }
}

#pragma endregion

#pragma region CONFIGURATION
/* initialize the board */
void BoardModule::begin() {
    LOGI("Loading configuration");
    JsonDocument doc;

    if (!openNVS(true)) {
        LOGE(
            "Error loading switch nvs partition in read only, trying to format it");
        if (!initNVS()) {
        LOGE("NVS INITIALIZATION FAILED");
        return;
        }
    }

    LOGV("Checking the schema version");
    // if I'm here NVS is surelly working, no more check...
    openNVS(true);
    int schemaVersion = nvs.getInt("schema", 0);
    LOGD("schema version is: %d", schemaVersion);

    if (schemaVersion < BOARD_SCHEMA_VERSION) {
        LOGW("Schema version: %d, new version: %d", schemaVersion, BOARD_SCHEMA_VERSION);
        switch (schemaVersion) {
        case 0:
        LOGI("upgrading from 0 to 1");
        updateNVS1();
        break;

        default:
        break;
        }
    }

    openNVS(true);
    identifier = nvs.getString("identifier", "TeslaBoard");
    locale = nvs.getString("locale", "en");
    LOGV("Board identifier: %s", identifier);
    LOGV("Board locale: %s", locale);


    closeNVS();
}

bool BoardModule::initNVS() {

  LOGW("Board nvs area will be formatted");
  if (!openNVS(false)) {
    LOGE("Unable to open the namespace with write rights, initialization failed");
    return false;
  }
  LOGI("namespace open or created, writing default parameters");

  nvs.putInt("schema", 0);
  
  closeNVS();
  return true;
}

void BoardModule::updateNVS1() {
  // this is the first schema, don't check if something already exist.
    openNVS(false);
    nvs.putInt("schema", 1);
    nvs.putString("identifier", "TeslaBoard");
    nvs.putString("locale", "en");
    closeNVS();
}

void BoardModule::getConfiguration(JsonObject dest){

    dest["identifier"] = identifier;
    dest["locale"] = locale;

}

void BoardModule::validateConfiguration(const JsonObject &toBeValidated, JsonObject response){

    response["reboot"] = false;

    JsonArray err = response["errors"].to<JsonArray>();

    if(!toBeValidated["identifier"].is<String>()){
        err.add("Identifier is not a string");
        LOGE("Identifier is not a String");
        return;
    }

    if(!toBeValidated["locale"].is<String>()){
        err.add("Locale is not a string");
        LOGE("Locale is not a string");
        return;
    }

    String tmp = toBeValidated["locale"].as<String>();

    if(tmp != "en" && tmp != "it" && tmp != "de" && tmp != "fr" && tmp != "es"){
        err.add("Locale don't exist");
        LOGE("Locale don't exist");
        return;
    }

    LOGI("Board Module Validated");
}

void BoardModule::storeConfiguration(JsonObject toBeStored){
    LOGI("Writing new configuration on the NVS");
    openNVS(false);

    identifier = toBeStored["identifier"].as<String>();
    locale = toBeStored["locale"].as<String>();

    nvs.putString("identifier", identifier);
    nvs.putString("locale", locale);

    closeNVS();

}

#pragma endregion CONFIGURATION

void BoardModule::loop(){
    
    if(millis() - ackMillis >= 60000){
        ackMillis = millis();
        upTime++;
    }
}


