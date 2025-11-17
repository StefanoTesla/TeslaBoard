#include "CoverCalibratorModule.h"
#include "esp_log.h"
#define LOG_TAG "CovCal"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

#pragma region nvsHandler

bool CoverCalibratorModule::openNVS(bool readOnly) {

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

void CoverCalibratorModule::closeNVS() {
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
/* initialize the dome */
void CoverCalibratorModule::begin(){
    LOGI("Loading configuration");
    JsonDocument doc;


    if(!openNVS(true)){
        LOGE("Error loading nvs partition, trying to format it");
        initNVS();
        if(!openNVS(true)){
            closeNVS();
            LOGE("Critical, unable to load nvs after initialization");
            return;
        };
    }else{LOGV("Namespace open without problem");}

    openNVS(true);

    moduleEnable = nvs.getBool("enable");
    uiOrder = nvs.getInt("order",1);
    identifier = nvs.getString("identifier","CoverCalibrator");

    if(!moduleEnable){
        LOGW("Module not enable, setup completed");
        closeNVS();
        return;
    }
    
    int schemaVersion = nvs.getInt("schema");
    LOGD("schema version is: %d", schemaVersion);

    if(schemaVersion < COVERC_SCHEMA_VERSION){
        LOGW("schema need an upgrade");
        switch (schemaVersion)
        {
            case 0:
                LOGI("upgrading from 0 to 1");
                updateNVS1();
                break;
            
            default:
                break;
        }
    }

    openNVS(true);

    /* basic data for CoverCalibratorModule is taken, module bening*/
    LOGI("deserialization of calibrator json configuration");
    String cfg = nvs.getString("calibrator","{}");
    LOGD("raw calibrator json is: %s",cfg.c_str());
    DeserializationError error = deserializeJson(doc, cfg);
    LOGD("calibrator deserialization ret val: %d 0=no error",error);

    if(!error){
        calibrator.begin(doc);
    }

    cfg.clear();
    cfg = nvs.getString("cover","{}");
    LOGD("raw cover json is: %s",cfg.c_str());
    error = deserializeJson(doc, cfg);
    LOGD("cover cover ret val: %d 0=no error",error);
    
    closeNVS();

    if(!error){
        cover.begin(doc);
    }

    LOGV("CoverC Begin finish");

}

void CoverCalibratorModule::updateNVS1(){
    openNVS(false);
    nvs.begin(COVERC_SCHEMA_NAME);
    nvs.putBool("enable",false);
    nvs.putInt("schema",COVERC_SCHEMA_VERSION);
    nvs.putInt("order",1);
    nvs.putString("calibrator","{}");
    nvs.putString("cover","{}");
    nvs.end();
    closeNVS();
}

bool CoverCalibratorModule::initNVS(){


    LOGI("NVS initialization begin");
    if(!openNVS(false)){
        LOGE("Unable to access to NVS, initialization failed");
        closeNVS();
        return false;
    };

    nvs.putBool("enable",false);
    nvs.putInt("order",1);
    nvs.putInt("schema",COVERC_SCHEMA_VERSION);
    nvs.putString("identifier","CoverCalibrator");
    nvs.putString("calibrator","{}");
    nvs.putString("cover","{}");
    closeNVS();

    return true;
}

bool CoverCalibratorModule::isEnable(){
    return moduleEnable;
}

void CoverCalibratorModule::getConfiguration(JsonObject dest){

    dest["enable"] = moduleEnable;
    dest["uiOrder"] = uiOrder;
    dest["identifier"] = identifier;
    JsonObject calibObj = dest["calibrator"].to<JsonObject>();
    calibrator.getConfiguration(calibObj);
    JsonObject coverObj = dest["cover"].to<JsonObject>();
    cover.getConfiguration(coverObj);

}

void CoverCalibratorModule::validateConfiguration(const JsonObject &toBeValidated, JsonObject response){

    response["reboot"] = false;

    JsonArray err = response["errors"].to<JsonArray>();
    if(!toBeValidated["enable"].is<bool>()){
        err.add("Enable is not a boolean");
        LOGE("Enable is not a boolean");
        return;
    }

    if(moduleEnable != toBeValidated["enable"]){
        response["reboot"] = true;
        LOGI("Enable is not like the actual one, reboot needed");
    }

    if(!toBeValidated["uiOrder"].is<int>()){
        err.add("Order is not a numeber");
        LOGE("UI order is not a number");
        return;
    }

    if(!toBeValidated["identifier"].is<String>()){
        err.add("Identifier is not a string");
        LOGE("Identifier is not a String");
        return;
    }

    if(!toBeValidated["enable"]){
        LOGI("Main module is not enable, stop validation");
        return;
    }

    LOGI("Main data validation ok, starting with calibrator data");

    if(!toBeValidated["calibrator"].is<JsonObject>()){
        err.add("Calibrator Configuration is missing");
        LOGE("Calibrator object is missing");
        return;
    }

    calibrator.validateConfiguration(toBeValidated["calibrator"],response);

    if(err.size()>0){
        return;
    }

    if(!toBeValidated["cover"].is<JsonObject>()){
        LOGE("Cover object is missing");
        err.add("Calibrator Configuration is missing");
        return;
    }

    cover.validateConfiguration(toBeValidated["cover"],response);

    if(err.size()>0){
        return;
    }

    rebootNeeded = response["reboot"].as<bool>();
}

void CoverCalibratorModule::storeConfiguration(JsonObject toBeStored){
    LOGI("Writing new configuration on the NVS");

    openNVS(false);

    bool inEnable = toBeStored["enable"].as<bool>();

    nvs.putBool("enable",inEnable);
    nvs.putInt("uiOrder",toBeStored["uiOrder"].as<int>());
    nvs.putInt("schema",COVERC_SCHEMA_VERSION);
    nvs.putString("identifier",toBeStored["identifier"].as<String>());
 
    closeNVS();

    /* apply only the changes that don't require a reboot */
    uiOrder = toBeStored["uiOrder"].as<int>();
    identifier = toBeStored["identifier"].as<String>();

    /* if module is not enable don't write anymore*/
    if(!inEnable ){
        LOGI("Main Module is not enable, writing new configuration done.");
        return;
    }
    
    LOGI("Writing calibrator config");
    calibrator.storeConfiguration(toBeStored["calibrator"],COVERC_SCHEMA_NAME);

    LOGI("Writing cover config");
    cover.storeConfiguration(toBeStored["cover"],COVERC_SCHEMA_NAME);

}

#pragma endregion CONFIGURATION

void CoverCalibratorModule::loop(){
    
    if(isEnable()){
        calibrator.loop();
        cover.loop();
    }
}


