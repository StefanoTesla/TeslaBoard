#include "Condition.h"
#include "esp_log.h"
#include "SwitchModule.h"
#undef LOG_TAG
#define LOG_TAG "Condition"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

Condition::ConditionStatusEnum Condition::evalutate() {
    LOGD("Evaluating condition: %s", Name);
//    status = ConditionStatusEnum::Unknow;

    int id = switches->findSwitchByUid(uniqueId);

    LOGD("Switch found with id: %d", id);
    if(id < 0){
        LOGE("Switch ID out of range");
        status = ConditionStatusEnum::Error;
        return status; }

    LOGD("Check if is expired: %d", id);
    if(switches->getIsExpired(id)){
        LOGE("Switch expired");
        status = ConditionStatusEnum::Error;
        return status;
    }
    LOGD("Not expired");
    int value = switches->getSwitchState(id);

    LOGD("Switch got this value: %d",value);
    ConditionStatusEnum result = ConditionStatusEnum::Unsafe;

    switch (checktype)
    {
    case ConditionCheckEnum::Less :
        LOGD("Less check type");
        if(value < refValue){
            LOGD("Condition status is safe");
            result = ConditionStatusEnum::Safe;
        }
        break;
    case ConditionCheckEnum::LessEqual :
        LOGD("Less or equal check type");
        if(value <= refValue){
            LOGD("Condition status is safe");
            result = ConditionStatusEnum::Safe;
        }
        break;
    case ConditionCheckEnum::Equal :
        LOGD("Equal check type");
        if(value == refValue){
            LOGD("Condition status is safe");
            result = ConditionStatusEnum::Safe;
        }
        break;
    case ConditionCheckEnum::GreaterEqual :
        LOGD("Greater or equal check type");
        if(value >= refValue){
            LOGD("Condition status is safe");
            result = ConditionStatusEnum::Safe;
        } 
        break;
    case ConditionCheckEnum::Greater :
        if(value > refValue){
            LOGD("Greater check type");
            LOGD("Condition status is safe");
            result = ConditionStatusEnum::Safe;
        }
        break;
    
    default:
        result = ConditionStatusEnum::Error;
        LOGE("Wrong check type selected");
        break;
    }

    status = result;
    LOGD("Returned status is= %d", status);
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