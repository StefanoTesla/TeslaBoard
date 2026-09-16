#ifndef CONDITION_H
#define CONDITION_H

#include <Arduino.h>
#include <NVSManager.h>
#include <ArduinoJson.h>
#include "esp_log.h"

class SwitchModule;

class Condition {

public:
    Condition() = default;
    void begin(const JsonDocument& condition);
    void setSwitchModule(SwitchModule* sw) { switches = sw; }
    void getConfiguration(JsonObject obj);
    const char* getName() { return Name; }
    int getStatus() { return status; }
    int getReferenceValue() { return refValue; }
    int getCheckType() { { return checktype; }}

    static void validateConfiguration(const JsonObject &obj, JsonObject response);
    static void copyJsonCfg(JsonObject obj,JsonObject dest);
    
    enum ConditionStatusEnum {
        Unknow = 0,
        Error,
        Safe,
        Unsafe
    };

    
    ConditionStatusEnum evalutate();




private:
    SwitchModule* switches = nullptr;

    char Name[31] = "";
    char uniqueId[9] = "";
    int refValue = 0;
    int tmpId;

    enum ConditionCheckEnum {
        Less = 0,
        LessEqual,
        Equal,
        GreaterEqual,
        Greater
    };

    ConditionStatusEnum status = ConditionStatusEnum::Unknow;

    ConditionCheckEnum checktype = ConditionCheckEnum::Less;
};
#endif