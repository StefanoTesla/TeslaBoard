#ifndef CONDITION_H
#define CONDITION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "esp_log.h"

class SwitchModule;

class Condition {

public:
    Condition() = default;
    void begin(const JsonDocument& condition);
    void setSwitchModule(SwitchModule* sw) { switches = sw; }
    void getConfiguration(JsonObject obj);
    const char* getName() const { return Name; }
    int getStatus() const {return static_cast<int>(status); }
    int getReferenceValue() const { return refValue; }
    int getCheckType() const { return static_cast<int>(checktype); }
    static void copyJsonCfg(JsonObjectConst obj,JsonObject dest);
    
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