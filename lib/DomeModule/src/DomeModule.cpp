#include "DomeModule.h"
#undef LOG_TAG
#define LOG_TAG "Dome"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

#pragma region nvsHandler

bool DomeModule::openNVS(bool readOnly) {

  switch (nvsStatus) {

    // nvs is closed, i need to open according by the readOnly
  case CLOSED:
    LOGV("NVS seems to be closed");
    if (nvs.begin(DOME_SCHEMA_NAME, readOnly)) {
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
      if (nvs.begin(DOME_SCHEMA_NAME, false)) {
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
      if (nvs.begin(DOME_SCHEMA_NAME, true)) {
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

void DomeModule::closeNVS() {
  if (nvsStatus != CLOSED) {
    nvs.end();
    nvsStatus = CLOSED;
    LOGV("NVS closed");
  } else {
    LOGV("NVS already closed");
  }
}

#pragma endregion

/* initialize the dome */
void DomeModule::begin(){
    LOGI("Loading configuration");
    JsonDocument doc;



    if(!openNVS(true)){
        LOGE("Error loading dome nvs partition, trying to format it");
        if(!initNVS()){
            LOGE("Critical, unable to loading dome nvs after initialization");
            return;
        };
    }

    openNVS(true);

    moduleEnable = nvs.getBool("enable");
    uiOrder = nvs.getInt("order",1);
    identifier = nvs.getString("identifier","Dome");

    if(!moduleEnable){
        LOGW("Module not enable, setup completed");
        closeNVS();
        return;
    }

    int schemaVersion = nvs.getInt("schema");
    LOGD("schema version is: %d", schemaVersion);

    if(schemaVersion < DOME_SCHEMA_VERSION){
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

    /* basic data for dome is taken, module bening*/

    LOGI("deserialization of shutter json configuration");
    String cfg = nvs.getString("shutter","{}");
    LOGD("raw shutter json is: %s",cfg.c_str());
    DeserializationError error = deserializeJson(doc, cfg);
    closeNVS();
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
    LOGI("upgrading NVS from 0 to 1");
    if(openNVS(false)){
        nvs.putBool("enable",false);
        nvs.putInt("schema",DOME_SCHEMA_VERSION);
        nvs.putInt("order",1);
        nvs.putString("shutter","{}");
        nvs.end();
    } else {
        LOGE("Unable to open the name space for the upgrade");
    }


}

bool DomeModule::initNVS(){

    LOGI("initNVS() initialization begin");
    if(!openNVS(false)){
        LOGE("initNVS() failed to open the namespace with write rights, initialization failed");
        closeNVS();
        return false;
    };

    LOGV("Storing default keys");
    nvs.putBool("enable",false);
    nvs.putInt("order",1);
    nvs.putInt("schema",1);
    nvs.putString("identifier","Dome");
    nvs.putString("shutter","{}");
    nvs.end();

    LOGI("initNVS() end");
    return true;
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