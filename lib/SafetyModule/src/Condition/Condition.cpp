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
    obj["uniqueId"] = uniqueId;
    obj["refValue"] = refValue;
    obj["ckType"] = static_cast<int>(checktype);

}

int Condition::validateConfiguration(const JsonObject &obj){


    const char* uniqueID = obj["uniqueID"].as<const char*>();
    int id = switches->findSwitchByUid(uniqueID);

    if(id < 0){ 
        return -1;
    }
    
    int swType = switches->getType(id);

    unsigned int ckType = obj["ckType"].as<unsigned int>();
    int comaprsionValue = obj["refValue"].as<int>();

    // error type
    // tens digit switch type 1 2 3 4 5
    // x0 unupported type (servo)
    // unit digit error type
    // x1 check type unvalid
    // x2 reference value incorrect

    switch (swType)
    {
    // digital pins
    case 1:
        if(ckType != 2){
            return -11;
        }
        if(comaprsionValue != 0 && comaprsionValue != 1){
            return -12;
        }
            break;

    case 2:
        
        if(ckType != 2){
            return -21;
        }

        if(comaprsionValue != 0 && comaprsionValue != 1){
            return -11;
        }
            break;

        //pwm
    case 3:
        if(comaprsionValue < 0 || comaprsionValue > 4095){

            return -31;
        }
        break;
        //pwm
    case 4:
            return -40;
        break;    


        //virtual
    case 5:
        if(comaprsionValue < INT_MIN || comaprsionValue > INT_MAX){
            return -51;
        }

    
    default:
        break;
    }

    
}

void Condition::copyJsonCfg(JsonObject obj,JsonObject dest){
    dest["name"]     = obj["name"];
    dest["uniqueId"] = obj["uniqueId"];
    dest["refValue"] = obj["refValue"];
    dest["ckType"]   = obj["ckType"];

}


#pragma endregion