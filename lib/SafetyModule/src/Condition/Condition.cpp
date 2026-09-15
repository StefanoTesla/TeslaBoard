#include "Condition.h"
#include "SwitchModule.h"
#undef LOG_TAG
#define LOG_TAG "Condition"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

Condition::ConditionStatusEnum Condition::evalutate() {

    status = Unknow;
    return status;
}
/*
Configuration Area
*/

#pragma region Configuration

void Condition::begin(const JsonDocument& doc){
    strlcpy(Name, doc["name"].as<const char*>(), sizeof(Name));
    strlcpy(uniqueId, doc["uniqueId"].as<const char*>(), sizeof(uniqueId));
}



void Condition::getConfiguration(JsonObject obj){

}

void Condition::validateConfiguration(const JsonObject &obj, JsonObject response){

    JsonArray err = response["errors"].to<JsonArray>();
    

}

void Condition::storeConfiguration(JsonObject conditionObject, const char* schema){

    tmpCfg.clear();

    String json;

    serializeJson(tmpCfg,json);

    NvsManager::getInstance().putString("shutter",json);

    tmpCfg.clear();
}


#pragma endregion