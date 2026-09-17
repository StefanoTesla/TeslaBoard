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
    String cfg;
    tmpCfg.clear();

    configuredConditions = NvsManager::getInstance().getInt("cfg_cnd", 0);

    if(configuredConditions == 0){
        return;
    }

    /* to dooo*/
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
    JsonArray storeConditions = tmpCfg["Conditions"].to<JsonArray>();
    JsonArray incomingConds = toBeValidated["Conditions"].as<JsonArray>();

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

        // digital input and digital output mus use equal and 0 or 1 as reference value
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

        storeConditions.add(inCondition);
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

    JsonArray validated = tmpCfg["Conditions"].as<JsonArray>();

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

    int id = -1;

    for (JsonObject inCond : validated) {
        ++id;

        JsonDocument oneDoc;
        JsonObject sanitized = oneDoc.to<JsonObject>();
        Condition::copyJsonCfg(inCond, sanitized);

        char key[10];
        snprintf(key, sizeof(key), "cnd%d", id);

        String json;
        serializeJson(sanitized, json);
        NvsManager::getInstance().putString(key, json);

        if (id < SAFETY_MAX_CONDITIONS) {
            conditions[id].begin(oneDoc);
        }
    }

    for (size_t i = id + 1; i < SAFETY_MAX_CONDITIONS; ++i) {
        char key[10];
        snprintf(key, sizeof(key), "cnd%d", i);
        NvsManager::getInstance().removeKey(key);
    }

    NvsManager::getInstance().putInt("cfg_cnd", id + 1);
    configuredConditions = id + 1;
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

    for (int i = 0; i < configuredConditions; i++)
    {
        status["name"]= conditions[i].getName();
        status["status"]= conditions[i].getStatus();
        status["refValue"]=conditions[i].getReferenceValue();
        status["checkType"]=conditions[i].getCheckType();
        
    }
    
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