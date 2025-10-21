#include "CoverCalibratorModule.h"
#include "esp_log.h"
#define LOG_TAG "CovCal"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

/* initialize the dome */
void CoverCalibratorModule::begin(){
    LOGI("Loading configuration");
    JsonDocument doc;
    Preferences pref;
    
    bool nvsEnded = false;

    if(!pref.begin(COVERC_SCHEMA_NAME,true)){
        LOGE("Error loading nvs partition, trying to format it");
        nvsEnded = true;
        initNVS();
        if(!pref.begin(COVERC_SCHEMA_NAME,true)){
            LOGE("Critical, unable to load nvs after initialization");
            return;
        };
    }else{LOGV("Namespace open without problem");}

    moduleEnable = pref.getBool("enable");
    uiOrder = pref.getInt("order",1);
    identifier = pref.getString("identifier","CoverCalibrator");

    if(!moduleEnable){
        LOGW("Module not enable, setup completed");
        pref.end();
        return;
    }
    
    int schemaVersion = pref.getInt("schema");
    LOGD("schema version is: %d", schemaVersion);

    if(schemaVersion < COVERC_SCHEMA_VERSION){
        nvsEnded = true;
        pref.end(); //since the upgrade operation required to open the nvs with write rigths I close it for reopen again in read only leater
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

    if(nvsEnded){
        if(!pref.begin(COVERC_SCHEMA_NAME)){
            moduleEnable=false;
            LOGE("Error loading dome nvs partition, stop");
            return;
        }
    } 

    /* basic data for CoverCalibratorModule is taken, module bening*/
    LOGI("deserialization of calibrator json configuration");
    String cfg = pref.getString("calibrator","{}");
    LOGD("raw calibrator json is: %s",cfg.c_str());
    DeserializationError error = deserializeJson(doc, cfg);
    LOGD("calibrator deserialization ret val: %d 0=no error",error);

    if(!error){
        calibrator.begin(doc);
    }

    cfg.clear();
    cfg = pref.getString("cover","{}");
    LOGD("raw cover json is: %s",cfg.c_str());
    error = deserializeJson(doc, cfg);
    LOGD("cover cover ret val: %d 0=no error",error);
    pref.end();

    if(!error){
        cover.begin(doc);
    }

}

void CoverCalibratorModule::updateNVS1(){
    Preferences pref;
    pref.begin(COVERC_SCHEMA_NAME);
    pref.putBool("enable",false);
    pref.putInt("schema",1);
    pref.putInt("order",COVERC_SCHEMA_VERSION);
    pref.putString("calibrator","{}");
    pref.putString("cover","{}");
    pref.end();
}

void CoverCalibratorModule::initNVS(){

    Preferences pref;

    LOGI("NVS initialization begin");
    if(!pref.begin(COVERC_SCHEMA_NAME, false)){
        LOGE("Unable to access to NVS, initialization failed");
        pref.end();
        return;
    };

    pref.putBool("enable",false);
    pref.putInt("order",1);
    pref.putInt("schema",1);
    pref.putString("identifier","CoverCalibrator");
    pref.putString("calibrator","{}");
    pref.putString("cover","{}");
    pref.end();

    Serial.println("NVS initialized");
}

bool CoverCalibratorModule::isEnable(){
    return moduleEnable;
}


void CoverCalibratorModule::loop(){
    
    if(isEnable()){
        calibrator.loop();
        cover.loop();
    }
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

}


void CoverCalibratorModule::storeConfiguration(JsonObject toBeStored){
    LOGI("Writing new configuration on the NVS");
    Preferences pref;

    pref.begin(COVERC_SCHEMA_NAME);

    bool inEnable = toBeStored["enable"].as<bool>();

    pref.putBool("enable",inEnable);
    pref.putInt("uiOrder",toBeStored["uiOrder"].as<int>());
    pref.putInt("schema",COVERC_SCHEMA_VERSION);
    pref.putString("identifier",toBeStored["identifier"].as<String>());
 
    pref.end();

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