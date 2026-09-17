#include "SafetyModule.h"
#include "SwitchModule.h"

#include "esp_log.h"
#define LOG_TAG "Safety"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

#pragma region Configuration
/* here we write additional data if nvs was empty*/
void SafetyModule::initSecondaryData() {
    NvsManager::getInstance().putInt("cfg_cnd", 0);
    for (size_t i = 0; i < SAFETY_MAX_CONDITIONS; i++) {
        char key[10];
        sprintf(key, "cnd%d", i);
        NvsManager::getInstance().removeKey(key);
    }
  NvsManager::getInstance().putInt("schema", 1);
}

/* here we load secondary data during the begin */
void SafetyModule::loadSecondaryData() {
    configuredConditions = NvsManager::getInstance().getInt("cfg_cnd", 0);

    if (configuredConditions == 0) {
        LOGI("No conditions configured");
        return;
    }

    if (configuredConditions > SAFETY_MAX_CONDITIONS) {
        LOGW("cfg_cnd=%u exceeds max (%u), clamping", (unsigned)configuredConditions, (unsigned)SAFETY_MAX_CONDITIONS);
        configuredConditions = SAFETY_MAX_CONDITIONS;
    }

    for (size_t i = 0; i < configuredConditions; i++) {
        char key[10];
        snprintf(key, sizeof(key), "cnd%d", i);

        String json = NvsManager::getInstance().getString(key, "");
        if (json.isEmpty()) {
            LOGE("Missing %s on NVS", key);
            continue;
        }

        tmpCfg.clear();
        DeserializationError err = deserializeJson(tmpCfg, json);
        if (err != DeserializationError::Ok) {
            LOGE("Error deserializing %s: %s", key, err.c_str());
            continue;
        }

        conditions[i].begin(tmpCfg);
    }

    LOGI("Loaded %u conditions", (unsigned)configuredConditions);
}


/* here we update the nvs when new schema is given */
bool SafetyModule::applySchemaUpgradeStep(uint16_t currentVersion) {
    LOGI("Applying schema upgrade step from version %u", currentVersion);

    if (!NvsManager::getInstance().openNVS(false, SAFETY_SCHEMA_NAME)) {
        LOGE("Unable to open board namespace for schema upgrade");
        return false;
    }

    switch (currentVersion) {
        case 0:
            NvsManager::getInstance().putString("identifier", "Safety");
            NvsManager::getInstance().putInt("schema", 1);
            NvsManager::getInstance().putInt("cfg_cnd", 0);
            NvsManager::getInstance().closeNVS();
            return true;

        default:
            LOGE("Unknown schema version %u for board upgrade", currentVersion);
            NvsManager::getInstance().closeNVS();
            return false;
    }
}

/* here we read secondary data during the get config */
void SafetyModule::appendSecondaryConfig(JsonObject dest) {
  JsonArray conditionsArray = dest["Conditions"].to<JsonArray>();
    for (size_t i = 0; i < configuredConditions; i++) {
        JsonObject cnd = conditionsArray.add<JsonObject>();
        conditions[i].getConfiguration(cnd);
  }
}

/* here the validation of secondary data when store configuration is called*/
bool SafetyModule::validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) {
    JsonArray err = response["errors"].to<JsonArray>();
    tmpCfg.clear();

    if (!toBeValidated["Conditions"].is<JsonArray>()) {
        err.add("Conditions is not an array");
        return false;
    }

    JsonArray incomingConds = toBeValidated["Conditions"].as<JsonArray>();

    if (incomingConds.size() > SAFETY_MAX_CONDITIONS) {
        JsonObject e = err.add<JsonObject>();
        e["error"] = "tooManyConditions";
        e["max"]   = SAFETY_MAX_CONDITIONS;
        return false;
    }

    int swId;
    int swType;
    int ckType;
    int refValue;
    int i = 0;

    for (JsonObject inCondition : incomingConds) {

        swId = -1;
        swType = -1;
        ckType = 0;

        swId = switchModule->findSwitchByUid(inCondition["uniqueId"].as<const char*>());

        if(swId < 0 || swId >= switchModule->maxConfigurableSwitches()){
            JsonObject e = err.add<JsonObject>();
            e["id"] = i;
            e["error"] = "swNotFound";
            break;
        }

        swType = switchModule->getType(swId);

        if(swType < 1 || swType > 5){
            JsonObject e = err.add<JsonObject>();
            e["id"] = i;
            e["error"] = "swTypeNotValid";
            break;
        }

        if (!inCondition["ckType"].is<int>()) {
            JsonObject e = err.add<JsonObject>();
            e["id"] = i;
            e["error"] = "ckTypeNotInt";
            break;
        }

        ckType = inCondition["ckType"].as<int>();

        if(ckType < 0 || ckType > 4){
            JsonObject e = err.add<JsonObject>();
            e["id"] = i;
            e["error"] = "ckTypeNotValid";
            break;
        }

        if (!inCondition["refValue"].is<int>()) {
            JsonObject e = err.add<JsonObject>();
            e["id"] = i;
            e["error"] = "refValueNotInt";
            break;
        }

        refValue = inCondition["refValue"].as<int>();

        if (!inCondition["name"].is<const char*>()) {
            JsonObject e = err.add<JsonObject>();
            e["id"]    = i;
            e["error"] = "nameMissing";
            break;
        }

        if(swType == 1 || swType == 2){
        
            if(ckType != 2){
                JsonObject e = err.add<JsonObject>();
                e["id"] = i;
                e["error"] = "digitalMustUseEqual";
                break;
            }
            
            if(refValue != 0 && refValue != 1){
                JsonObject e = err.add<JsonObject>();
                e["id"] = i;
                e["error"] = "digitalValueForbitten";
                break;
            }

        }

        //pwm refvalue max is 4095
        if(swType == 3){
            if(refValue < 0 || refValue > 4095){
                JsonObject e = err.add<JsonObject>();
                e["id"] = i;
                e["error"] = "pwmValueForbitten";
                break;
            }

        }

        //servo are not used in safety
        if(swType == 4){
            JsonObject e = err.add<JsonObject>();
            e["id"] = i;
            e["error"] = "swServoNotUsable";
            break;
        }

        i+=1;
    }

    if(err.size() != 0){
        tmpCfg.clear();
        return false;
    }

    return true;
}

/* here we store secondary data during the save config */
void SafetyModule::storeSecondaryConfig(const JsonObject& toBeStored) {

    JsonArray validated = toBeStored["Conditions"].as<JsonArray>();

    if (validated.isNull() || validated.size() == 0) {
        for (size_t i = 0; i < SAFETY_MAX_CONDITIONS; ++i) {
            char key[10];
            snprintf(key, sizeof(key), "cnd%u", i);
            NvsManager::getInstance().removeKey(key);
        }
        NvsManager::getInstance().putInt("cfg_cnd", 0);
        configuredConditions = 0;
        return;
    }

    int written = -1;

    for (JsonObject inCond : validated) {
        ++written;
        if (written >= SAFETY_MAX_CONDITIONS) {
            LOGE("Too many conditions to store!");
            break;
        }

        JsonDocument oneDoc;
        JsonObject sanitized = oneDoc.to<JsonObject>();
        Condition::copyJsonCfg(inCond, sanitized);

        char key[10];
        snprintf(key, sizeof(key), "cnd%d", written);

        conditions[written].begin(oneDoc);

        String json;
        serializeJson(sanitized, json);
        NvsManager::getInstance().putString(key, json);

    }

    for (size_t i = written + 1; i < SAFETY_MAX_CONDITIONS; ++i) {
        char key[10];
        snprintf(key, sizeof(key), "cnd%d", i);
        NvsManager::getInstance().removeKey(key);
    }

    NvsManager::getInstance().putInt("cfg_cnd", written + 1);
    configuredConditions = written + 1;
}

#pragma endregion


void SafetyModule::loop(){
    
    if(isEnable()){
        status = SafetyStatusEnum::Safe;
        for (size_t i = 0; i < configuredConditions; i++)
        {
            if(conditions[i].evalutate() != Condition::ConditionStatusEnum::Safe){
                status = SafetyStatusEnum::Unsafe;
            }
        }
    }
}


void SafetyModule::reportConditionState(int id, JsonObject status){
    if (id < 0 || id >= static_cast<int>(configuredConditions)) return;
    status["name"]      = conditions[id].getName();
    status["status"]    = conditions[id].getStatus();
    status["refValue"]  = conditions[id].getReferenceValue();
    status["checkType"] = conditions[id].getCheckType();
}


#pragma region SERIAL

SafetyModule::SafetySerialCommand SafetyModule::parseCommand(const char* cmd) {
    if (strcmp(cmd, "DEVICE_STATE") == 0)     return SafetySerialCommand::DeviceState;
    if (strcmp(cmd, "IS_SAFE") == 0)     return SafetySerialCommand::IsSafe;
    if (strcmp(cmd, "DESC") == 0)           return SafetySerialCommand::Desc;
    if (strcmp(cmd, "INT_VRS") == 0)        return SafetySerialCommand::IntVersion;
    if (strcmp(cmd, "NAME") == 0)           return SafetySerialCommand::Name;
    if (strcmp(cmd, "SUP_ACTIONS") == 0)    return SafetySerialCommand::SupportedActions;
    if (strcmp(cmd, "ACTION") == 0)         return SafetySerialCommand::Action;
    if (strcmp(cmd, "CMD_BLIND") == 0)      return SafetySerialCommand::CmdBlind;
    if (strcmp(cmd, "CMD_BOOL") == 0)       return SafetySerialCommand::CmdBool;
    if (strcmp(cmd, "CMD_STRING") == 0)     return SafetySerialCommand::CmdString;
    if (strcmp(cmd, "CONNECT") == 0)        return SafetySerialCommand::Connect;
    if (strcmp(cmd, "CONNECTING") == 0)        return SafetySerialCommand::Connecting;
    if (strcmp(cmd, "DISCONNECT") == 0)     return SafetySerialCommand::Disconnect;
    if (strcmp(cmd, "CONNECTED") == 0)     return SafetySerialCommand::Connected;

  LOGI("Command not found: %s",cmd);
  return SafetySerialCommand::Unknown;
}


bool SafetyModule::handlePacket(char* payload, Stream& out) {
    char* saveptr = nullptr;
    char* cmd = strtok_r(payload, ":", &saveptr);

    if (cmd == nullptr) {
        out.print("<CC:ERR:BAD_CMD:NULLPTR>");
        return false;
    }

    SafetySerialCommand command;
    LOGI("Command recived: %s", cmd);
    command = parseCommand(cmd);

    if (command == SafetySerialCommand::Unknown) {
        out.print("<CC:ERR:BAD_CMD:UNKNOW>");
        return false;
    }

    switch (command) {

        case SafetySerialCommand::Desc:
            out.print("<SF:OK:");
            out.print(getIdentifier());
            out.print("- TeslaBoard via USB>");
            return true;

        case SafetySerialCommand::IntVersion:
            out.print("<SF:2>");
            return true;

        case SafetySerialCommand::Name:
            out.print("<SF:");
            out.print(getIdentifier());
            out.print("- TeslaBoard>");
            return true;

        case SafetySerialCommand::Connect:
        case SafetySerialCommand::Disconnect:
            out.print("<SF:OK>");
            return true;

        case SafetySerialCommand::Connected:
            out.print("<SF:true>");
            return true;

        case SafetySerialCommand::Connecting:
            out.print("<SF:false>");
            return true;

        case SafetySerialCommand::SupportedActions:
            out.print("<SF:>");
            return true;

        case SafetySerialCommand::Action:
        case SafetySerialCommand::CmdBlind:
        case SafetySerialCommand::CmdBool:
        case SafetySerialCommand::CmdString:
            out.print("<SF:ERR:NOT_IMPL>");
            return true;

        case SafetySerialCommand::DeviceState:
            out.print("<SF:");
            if(isSafe()){
                out.print("1");
            } else {
                out.print("0");
            }
            out.print(">");
            return true;
    }


    // Se siamo qui, il comando non è stato gestito
    LOGI("Command not handled: %s", cmd);
    out.print("<SF:ERR:BAD_CMD:DRIVER_EXC>");
    return false;

    #pragma endregion
}