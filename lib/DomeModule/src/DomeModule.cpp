#include "DomeModule.h"
#undef LOG_TAG
#define LOG_TAG "Dome"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

/* initialize the dome */
void DomeModule::begin(){
    LOGI("Loading configuration");
    JsonDocument doc;
    Preferences pref;
    
    if(!pref.begin(DOME_SCHEMA_NAME,true)){
        LOGE("Error loading dome nvs partition, trying to format it");
        initNVS();
        if(!pref.begin(DOME_SCHEMA_NAME)){
            LOGE("Critical, unable to loading dome nvs after initialization");
            return;
        };
    };

    moduleEnable = pref.getBool("enable");
    uiOrder = pref.getInt("order",1);
    identifier = pref.getString("identifier","Dome");

    if(!moduleEnable){
        LOGW("Module not enable, setup completed");
        pref.end();
        return;
    }

    int schemaVersion = pref.getInt("schema");
    LOGD("schema version is: %d", schemaVersion);



    if(schemaVersion < DOME_SCHEMA_VERSION){
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

    if(!pref.begin(DOME_SCHEMA_NAME)){
        moduleEnable=false;
        LOGE("Error loading dome nvs partition, stop");
        return;
    };
    /* basic data for dome is taken, module bening*/

    LOGI("deserialization of shutter json configuration");
    String cfg = pref.getString("shutter","{}");
    LOGD("raw shutter json is: %s",cfg);
    DeserializationError error = deserializeJson(doc, cfg);
    pref.end();
    LOGD("shutter deserialization ret val: %d 0=no error",error);

    if(!error){
        shutter.begin(doc);
    }

    if(shutter.isEnable()){
        LOGI("Shutter setup finish with no issue");
        moduleEnable = true;
    } else {
        LOGE("Shutter is not enable, olso disable Dome module");
        moduleEnable = false;
    }

}

void DomeModule::updateNVS1(){
    Preferences pref;
    LOGI("upgrading NVS from 0 to 1");
    pref.end();
    pref.begin(DOME_SCHEMA_NAME);
    pref.putBool("enable",false);
    pref.putInt("schema",1);
    pref.putInt("order",DOME_SCHEMA_VERSION);
    pref.putString("shutter","{}");
    pref.end();

}

void DomeModule::initNVS(){
    Preferences pref;
    LOGI("NVS initialization begin");
    if(!pref.begin(DOME_SCHEMA_NAME, false)){
        LOGE("Unable to read the NVS page, initialization failed");
        pref.end();
        return;
    };

    pref.putBool("enable",false);
    pref.putInt("order",1);
    pref.putInt("schema",1);
    pref.putString("identifier","Dome");
    pref.putString("shutter","{}");
    pref.end();

    LOGI("NVS initializated");
}

bool DomeModule::isEnable(){
    return moduleEnable;
}


void DomeModule::loop(){
    
    if(isEnable()){
        shutter.loop();
    }
}


void DomeModule::getConfiguration(JsonObject dest){

    dest["enable"] = moduleEnable;
    dest["uiOrder"] = uiOrder;
    dest["identifier"] = identifier;
    JsonObject shutterObj = dest["shutter"].to<JsonObject>();
    shutter.getConfiguration(shutterObj);

}


void DomeModule::validateConfiguration(const JsonObject &toBeValidated, JsonObject response){
    LOGI("data validation");
    response["reboot"] = false;

    JsonArray err = response["errors"].to<JsonArray>();
    if(!toBeValidated["enable"].is<bool>()){
        LOGE("enable don't exist or is not a boolean");
        err.add("Enable is not a boolean");
        return;
    }


    if(moduleEnable != toBeValidated["enable"]){
        LOGW("enable is not the same as actual, reboot requested");
        response["reboot"] = true;
    }

    if(!toBeValidated["uiOrder"].is<int>()){
        LOGE("order for ui don't exist or is not a integer");
        err.add("Order is not a numeber");
        return;
    }

    if(!toBeValidated["identifier"].is<String>()){
        LOGE("board identifier don't exist or is not a string");
        err.add("Identifier is not a string");
        return;
    }

    if(!toBeValidated["shutter"].is<JsonObject>()){
        LOGE("Shutter Configuration is missing or is not a json object");
        err.add("Shutter Configuration is missing");
        return;
    }
    
    if(!toBeValidated["enable"]){
        LOGI("Main module is not enable, stop validation");
        return;
    }

    LOGI("Main data validation ok, starting with shutter data");

    shutter.validateConfiguration(toBeValidated["shutter"],response);

}

void DomeModule::storeConfiguration(JsonObject toBeStored){
    LOGI("Writing new configuration on the NVS");
    Preferences pref;

    pref.begin(DOME_SCHEMA_NAME);

    bool inEnable = toBeStored["enable"].as<bool>();

    pref.putBool("enable",inEnable);
    pref.putInt("uiOrder",toBeStored["uiOrder"].as<int>());
    pref.putInt("schema",DOME_SCHEMA_VERSION);
    pref.putString("identifier",toBeStored["identifier"].as<String>());
 
    pref.end();

    /* apply only the changes that don't require a reboot */
    LOGI("Applying data dont require a reboot");
    uiOrder = toBeStored["uiOrder"].as<int>();
    identifier = toBeStored["identifier"].as<String>();

    /* if module is not enable don't write anymore*/
    if(!inEnable ){
        LOGI("Main Module is not enable, writing new configuration done.");
        return;
    }
    LOGI("Main config done, start with shutter");
    shutter.storeConfiguration(toBeStored["shutter"],DOME_SCHEMA_NAME);

}