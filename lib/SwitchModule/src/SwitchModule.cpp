#include "SwitchModule.h"
#undef LOG_TAG
#define LOG_TAG "Switch"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)


#pragma region Configuration
/* initialize the dome */
void SwitchModule::begin(){
    LOGI("Loading configuration");
    JsonDocument doc;
    Preferences pref;
    
    bool nvsStop = false;

    if(!pref.begin(SWITCH_SCHEMA_NAME)){
        LOGE("Error loading dome nvs partition, trying to format it");
        pref.end();
        initNVS();
        if(!pref.begin(SWITCH_SCHEMA_NAME)){
            LOGE("Critical, unable to loading dome nvs after initialization");
            return;
        };
    }else{LOGV("Namespace open without problem");}

    moduleEnable = pref.getBool("enable");
    uiOrder = pref.getInt("order",1);
    identifier = pref.getString("identifier","Dome");

    if(!moduleEnable){
        LOGW("Module not enable, setup completed");
        pref.end();
        LOGV("pref.end");
        return;
    }

    int schemaVersion = pref.getInt("schema");
    LOGD("schema version is: %d", schemaVersion);



    if(schemaVersion < SWITCH_SCHEMA_VERSION){
        LOGV("pref.end");
        nvsStop = true;
        pref.end(); //since the upgrade operation required to open the nvs with write rigths I close it for reopen again in read only leater
        nvsStop = true;
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

    if(nvsStop){
        LOGV("nvs was stopped previusly, reopening it");
        if(!pref.begin(SWITCH_SCHEMA_NAME)){
            moduleEnable=false;
            LOGE("Error loading dome nvs partition, stop");
            return;
        };
    }

    /* basic data for is taken, module bening*/


}

void SwitchModule::updateNVS1(){
    Preferences pref;
    LOGI("upgrading NVS from 0 to 1");
    if(pref.begin(DOME_SCHEMA_NAME)){
        pref.putBool("enable",false);
        pref.putInt("schema",DOME_SCHEMA_VERSION);
        pref.putInt("order",1);
        pref.putString("shutter","{}");
        pref.end();
    } else {
        LOGE("Unable to open the name space for the upgrade");
    }


}

void SwitchModule::initNVS(){
    Preferences pref;
    LOGI("initNVS() initialization begin");
    if(!pref.begin(SWITCH_SCHEMA_NAME, false)){
        LOGE("initNVS() failed to open the namespace with write rights, initialization failed");
        pref.end();
        return;
    };

    LOGV("Storing default keys");
    pref.putBool("enable",false);
    pref.putInt("order",1);
    pref.putInt("schema",1);
    pref.putString("identifier","Dome");
    pref.putString("switches","{}");
    pref.end();

    LOGI("initNVS() end");
}


void SwitchModule::getConfiguration(JsonObject dest){

    dest["enable"] = moduleEnable;
    dest["uiOrder"] = uiOrder;
    dest["identifier"] = identifier;
    JsonObject Switches = dest["switches"].to<JsonObject>();


}


void SwitchModule::validateConfiguration(const JsonObject &toBeValidated, JsonObject response){
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

    if(!toBeValidated["enable"]){
        LOGI("Main module is not enable, stop validation");
        return;
    }

    LOGI("Main data validation ok, starting with shutter data");


}

void SwitchModule::storeConfiguration(JsonObject toBeStored){
    LOGI("Writing new configuration on the NVS");
    Preferences pref;

    pref.begin(SWITCH_SCHEMA_NAME);

    bool inEnable = toBeStored["enable"].as<bool>();

    pref.putBool("enable",inEnable);
    pref.putInt("uiOrder",toBeStored["uiOrder"].as<int>());
    pref.putInt("schema",SWITCH_SCHEMA_VERSION);
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

}


#pragma endregion


bool SwitchModule::isEnable(){
    return moduleEnable;
}


void SwitchModule::loop(){
    
    if(isEnable()){
        shutter.loop();
    }
}

