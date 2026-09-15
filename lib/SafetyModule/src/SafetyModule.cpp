#include "SafetyModule.h"

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
    /* to dooo*/
}


/* here the validation of secondary data when store configuration is called*/
bool SafetyModule::validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) {
  JsonArray err = response["errors"].as<JsonArray>();

    /* to dooo*/

  if(err.size()>0){
      return false;
  }

  rebootNeeded = response["reboot"].as<bool>();

  return err.size() == 0;
}

/* here we store secondary data during the save config */
void SafetyModule::storeSecondaryConfig(const JsonObject &toBeStored) {

    /* to dooo*/
}


#pragma endregion


void SafetyModule::loop(){
    
    if(isEnable()){

    }
}


void SafetyModule::reportConditionState(int id, JsonObject status){

    for (int i = 0; i < conditionCount; i++)
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