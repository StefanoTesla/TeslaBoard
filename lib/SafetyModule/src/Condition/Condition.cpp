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

    int id = switches->findSwitchByUid(uniqueId);

    if(id < 0){ return Unknow; }

    int value = switches->getSwitchState(id);

    status = ConditionStatusEnum::Unsafe;
    switch (checktype)
    {
    case ConditionCheckEnum::Less :
        if(value < refValue){
            status = ConditionStatusEnum::Safe;
        }
        break;
    case ConditionCheckEnum::LessEqual :
        if(value <= refValue){
            status = ConditionStatusEnum::Safe;
        }
        break;
    case ConditionCheckEnum::Equal :
        if(value == refValue){
            status = ConditionStatusEnum::Safe;
        }
        break;
    case ConditionCheckEnum::GreaterEqual :
        if(value >= refValue){
            status = ConditionStatusEnum::Safe;
        } 
        break;
    case ConditionCheckEnum::Greater :
        if(value > refValue){
            status = ConditionStatusEnum::Safe;
        }
        break;
    
    default:
        status = ConditionStatusEnum::Error;
        break;
    }

    return status;
}
/*
Configuration Area
*/

#pragma region Configuration

void Condition::begin(const JsonDocument& doc){
    strlcpy(Name, doc["name"].as<const char*>(), sizeof(Name));
    strlcpy(uniqueId, doc["uniqueId"].as<const char*>(), sizeof(uniqueId));
    refValue = doc["refValue"].as<int>();
    int ckType = doc["ckTy"].as<int>();
    checktype = static_cast<ConditionCheckEnum>(ckType);
    tmpId = switches->findSwitchByUid(uniqueId);
}

void Condition::getConfiguration(JsonObject obj){

    obj["name"] = Name;
    obj["uniqueID"] = uniqueId;
    obj["refValue"] = refValue;
    obj["ckType"] = static_cast<int>(checktype);

}

void Condition::validateConfiguration(const JsonObject &obj, JsonObject response){

    JsonArray err = response["errors"].to<JsonArray>();

    int id = obj["uniqueId"].as<int>();
    

}

void Condition::copyJsonCfg(JsonObject src, JsonObject dest) {
  dest["name"] = src["name"];
  dest["uniqueId"] = src["uniqueId"];
  dest["refValue"] = src["refValue"];
  dest["ckType"] = src["ckType"];

}

#pragma endregion