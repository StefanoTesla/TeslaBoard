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

    if(id < 0){ return Error; }

    if(switches->getIsExpired(id)){
        return Error;
    }

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
    const char* n = doc["name"].as<const char*>();
    strlcpy(Name, n ? n : "", sizeof(Name));

    const char* u = doc["uniqueId"].as<const char*>();
    strlcpy(uniqueId, u ? u : "", sizeof(uniqueId));

    refValue = doc["refValue"].as<int>();
    checktype = static_cast<ConditionCheckEnum>(doc["ckType"].as<int>());

}

void Condition::getConfiguration(JsonObject obj){

    obj["name"] = Name;
    obj["uniqueId"] = uniqueId;
    obj["refValue"] = refValue;
    obj["ckType"] = static_cast<int>(checktype);

}

void Condition::copyJsonCfg(JsonObjectConst obj,JsonObject dest){
    dest["name"]     = obj["name"];
    dest["uniqueId"] = obj["uniqueId"];
    dest["refValue"] = obj["refValue"];
    dest["ckType"]   = obj["ckType"];
}


#pragma endregion