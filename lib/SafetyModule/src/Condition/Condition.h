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
    void validateConfiguration(const JsonObject &obj, JsonObject response);
    void storeConfiguration(JsonObject toBeStored,const char* schema);
    void getConfiguration(JsonObject obj);

    enum ConditionStatusEnum {
        Unknow = 0,
        Error,
        Safe,
        Unsafe
    };

    ConditionStatusEnum status = ConditionStatusEnum::Unknow;

    enum ConditionCheckEnum {
        Less = 0,
        LessEqual,
        Equal,
        GreaterEqual,
        Greater
    };

    ConditionCheckEnum checktype = ConditionCheckEnum::Less;

    ConditionStatusEnum evalutate();

private:
    JsonDocument tmpCfg;
    SwitchModule* switches = nullptr;

    char Name[31] = "";
    char uniqueId[9] = "";
};
#endif