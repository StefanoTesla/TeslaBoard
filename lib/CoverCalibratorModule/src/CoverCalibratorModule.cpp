#include "CoverCalibratorModule.h"
#include "esp_log.h"
#undef LOG_TAG
#define LOG_TAG "CovCal"
#ifdef COVER_CALIBRATOR_LOG
  #define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
  #define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
  #define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
  #define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
  #define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)
#else
  #define LOGV(...) do {} while (0)
  #define LOGD(...) do {} while (0)
  #define LOGI(...) do {} while (0)
  #define LOGW(...) do {} while (0)
  #define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)  // gli errori restano
#endif

#pragma region Configuration
/* here we write additional data if nvs was empty*/
void CoverCalibratorModule::initSecondaryData() {
    NvsManager::getInstance().putString("calibrator", "{}");
    NvsManager::getInstance().putString("cover", "{}");
}

/* here we load secondary data during the begin */
void CoverCalibratorModule::loadSecondaryData() {
    String cfg;
    tmpCfg.clear();

    /* load calibrator data */
    LOGI("deserialization of calibrator json configuration");
    cfg = NvsManager::getInstance().getString("calibrator","{}");
    LOGD("raw calibrator json is: %s",cfg.c_str());
    DeserializationError error = deserializeJson(tmpCfg, cfg);
    LOGD("calibrator deserialization ret val: %d 0=no error",error);
    if(!error){
        calibrator.begin(tmpCfg);
    }

    /* load cover data */
    cfg = NvsManager::getInstance().getString("cover","{}");
    LOGD("raw cover json is: %s",cfg.c_str());
    error = deserializeJson(tmpCfg, cfg);
    LOGD("cover cover ret val: %d 0=no error",error);

    if(!error){
        cover.begin(tmpCfg);
    }
    tmpCfg.clear();


    if(!cover.isEnable() && !calibrator.isEnable()){
      LOGD("Cover and Calibrator aren't enabled, going to disable the main module");
      moduleEnable = false;
    }
}


/* here we update the nvs when new schema is given */
bool CoverCalibratorModule::applySchemaUpgradeStep(uint16_t currentVersion) {
    LOGI("Applying schema upgrade step from version %u", currentVersion);

    if (!NvsManager::getInstance().openNVS(false, COVERC_SCHEMA_NAME)) {
        LOGE("Unable to open board namespace for schema upgrade");
        return false;
    }

    switch (currentVersion) {
        case 0:
            NvsManager::getInstance().putString("identifier", "CoverC");
            NvsManager::getInstance().putInt("schema", 1);
            NvsManager::getInstance().closeNVS();
            return true;

        default:
            LOGE("Unknown schema version %u for board upgrade", currentVersion);
            NvsManager::getInstance().closeNVS();
            return false;
    }
}

/* here we read secondary data during the get config */
void CoverCalibratorModule::appendSecondaryConfig(JsonObject dest) {
    JsonObject calibObj = dest["calibrator"].to<JsonObject>();
    calibrator.getConfiguration(calibObj);
    JsonObject coverObj = dest["cover"].to<JsonObject>();
    cover.getConfiguration(coverObj);
}


/* here the validation of secondary data when store configuration is called*/
bool CoverCalibratorModule::validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) {
  JsonArray err = response["errors"].as<JsonArray>();

  if(!toBeValidated["calibrator"].is<JsonObject>()){
      err.add("Calibrator Configuration is missing");
      LOGE("Calibrator object is missing");
      return false;
  }

  calibrator.validateConfiguration(toBeValidated["calibrator"],response);

  if(err.size()>0){
      return false;
  }

  if(!toBeValidated["cover"].is<JsonObject>()){
      LOGE("Cover object is missing");
      err.add("Calibrator Configuration is missing");
      return false;
  }

  cover.validateConfiguration(toBeValidated["cover"],response);

  if(err.size()>0){
      return false;
  }

  rebootNeeded = response["reboot"].as<bool>();

  return err.size() == 0;
}

/* here we store secondary data during the save config */
void CoverCalibratorModule::storeSecondaryConfig(const JsonObject &toBeStored) {
    LOGI("Writing calibrator config");
    calibrator.storeConfiguration(toBeStored["calibrator"]);

    LOGI("Writing cover config");
    cover.storeConfiguration(toBeStored["cover"]);
}


#pragma endregion


void CoverCalibratorModule::loop(){
    
    if(isEnable()){
        if(calibrator.isEnable()){
            calibrator.loop();
        }
        if(cover.isEnable()){
            cover.loop();
        }
    }
}



/* SERIAL MANAGER */
CoverCalibratorModule::CCSerialCommand CoverCalibratorModule::parseCommand(const char* cmd) {
    if (strcmp(cmd, "DEVICE_STATE") == 0)     return CCSerialCommand::DeviceState;
    if (strcmp(cmd, "CAL_STATE") == 0)     return CCSerialCommand::CalibratorState;
    if (strcmp(cmd, "COV_STATE") == 0)     return CCSerialCommand::CoverState;
    if (strcmp(cmd, "CAL_BRI") == 0)     return CCSerialCommand::Brightness;
    if (strcmp(cmd, "CAL_MAX_BRI") == 0)     return CCSerialCommand::MaxBrightness;
    if (strcmp(cmd, "CAL_CHANGING") == 0)     return CCSerialCommand::CalibratorChanging;
    if (strcmp(cmd, "CAL_OFF") == 0)     return CCSerialCommand::CalibratorOff;
    if (strcmp(cmd, "CAL_ON") == 0)     return CCSerialCommand::CalibratorOn;
    if (strcmp(cmd, "COV_MOVING") == 0)     return CCSerialCommand::CoverMoving;
    if (strcmp(cmd, "COV_OPEN") == 0)     return CCSerialCommand::OpenCover;
    if (strcmp(cmd, "COV_CLOSE") == 0)     return CCSerialCommand::CloseCover;
    if (strcmp(cmd, "COV_HALT") == 0)     return CCSerialCommand::HaltCover;
    if (strcmp(cmd, "DESC") == 0)           return CCSerialCommand::Desc;
    if (strcmp(cmd, "INT_VRS") == 0)        return CCSerialCommand::IntVersion;
    if (strcmp(cmd, "NAME") == 0)           return CCSerialCommand::Name;
    if (strcmp(cmd, "SUP_ACTIONS") == 0)    return CCSerialCommand::SupportedActions;
    if (strcmp(cmd, "ACTION") == 0)         return CCSerialCommand::Action;
    if (strcmp(cmd, "CMD_BLIND") == 0)      return CCSerialCommand::CmdBlind;
    if (strcmp(cmd, "CMD_BOOL") == 0)       return CCSerialCommand::CmdBool;
    if (strcmp(cmd, "CMD_STRING") == 0)     return CCSerialCommand::CmdString;
    if (strcmp(cmd, "CONNECT") == 0)        return CCSerialCommand::Connect;
    if (strcmp(cmd, "CONNECTING") == 0)        return CCSerialCommand::Connecting;
    if (strcmp(cmd, "DISCONNECT") == 0)     return CCSerialCommand::Disconnect;
    if (strcmp(cmd, "CONNECTED") == 0)     return CCSerialCommand::Connected;

  LOGI("Command not found: %s",cmd);
  return CCSerialCommand::Unknown;
}


bool CoverCalibratorModule::handlePacket(char* payload, Stream& out) {
    char* saveptr = nullptr;
    char* cmd = strtok_r(payload, ":", &saveptr);

    if (cmd == nullptr) {
        out.print("<CC:ERR:BAD_CMD:NULLPTR>");
        return false;
    }

    CCSerialCommand command;
    LOGI("Command recived: %s", cmd);
    command = parseCommand(cmd);

    if (command == CCSerialCommand::Unknown) {
        out.print("<CC:ERR:BAD_CMD:UNKNOW>");
        return false;
    }

    switch (command) {

        case CCSerialCommand::Desc:
            out.print("<CC:OK:");
            out.print(getIdentifier());
            out.print("- TeslaBoard via USB>");
            return true;

        case CCSerialCommand::IntVersion:
            out.print("<CC:2>");
            return true;

        case CCSerialCommand::Name:
            out.print("<CC:");
            out.print(getIdentifier());
            out.print("- TeslaBoard>");
            return true;

        case CCSerialCommand::Connect:
        case CCSerialCommand::Disconnect:
            out.print("<CC:OK>");
            return true;

        case CCSerialCommand::Connected:
            out.print("<CC:true>");
            return true;

        case CCSerialCommand::Connecting:
            out.print("<CC:false>");
            return true;

        case CCSerialCommand::SupportedActions:
            out.print("<CC:>");
            return true;

        case CCSerialCommand::Action:
        case CCSerialCommand::CmdBlind:
        case CCSerialCommand::CmdBool:
        case CCSerialCommand::CmdString:
            out.print("<CC:ERR:NOT_IMPL>");
            return true;

        case CCSerialCommand::DeviceState:
            out.print("<CC:");
            out.print(cover.getStatus());
            out.print(",");
            out.print(cover.getStatus() == 2 ? 1 : 0);
            out.print(",");
            out.print(calibrator.getStatus());
            out.print(",0,"); // calibratorChanging always at 0 pwm is update immidiatly
            out.print(calibrator.getBrightness());
            out.print(",");
            out.print(calibrator.getMaxBrightness());
            out.print(">");
            return true;
    }

    #pragma region Calibrator

    switch (command) {

        case CCSerialCommand::CalibratorState:
            out.print("<CC:");
            out.print(calibrator.getStatus());
            out.print(">");
            return true;

        case CCSerialCommand::CalibratorChanging:
            out.print("<CC:false>");
            return true;

        case CCSerialCommand::CalibratorOff:
            if (!calibrator.isEnable()) {
                out.print("<CC:ERR:NOT_ENABLE>");
                return false;
            }
            calibrator.setBrightness(0);
            out.print("<CC:OK>");
            return true;

        case CCSerialCommand::CalibratorOn:
        {
            if (!calibrator.isEnable()) {
                out.print("<CC:ERR:NOT_ENABLE>");
                return false;
            }

            char* chBrightness = strtok_r(nullptr, ":", &saveptr);
            if (chBrightness == nullptr || *chBrightness == '\0') {
                out.print("<CC:ERR:BAD_CMD:NO_BRI>");
                return false;
            }

            char* endPtr = nullptr;
            long val = strtol(chBrightness, &endPtr, 10);

            if (*endPtr != '\0') {
                out.print("<CC:ERR:BAD_CMD:BRI_MALFORMED>");
                return false;
            }

            if (val < 0 || val > calibrator.getMaxBrightness()) {
                out.print("<CC:ERR:BRIGHT_OUT_OF_RANGE>");
                return false;
            }

            int brightness = static_cast<int>(val);
            LOGI("Brightness requested: %d", brightness);
            calibrator.setBrightness(brightness);
            out.print("<CC:OK>");
            return true;
        }

        case CCSerialCommand::Brightness:
            out.print("<CC:");
            out.print(calibrator.getBrightness());
            out.print(">");
            return true;

        case CCSerialCommand::MaxBrightness:
            out.print("<CC:");
            out.print(calibrator.getMaxBrightness());
            out.print(">");
            return true;
    }

    #pragma endregion

    #pragma region Cover

    switch (command) {

        case CCSerialCommand::CoverState:
            out.print("<CC:");
            out.print(cover.getStatus());
            out.print(">");
            return true;

        case CCSerialCommand::CoverMoving:
            out.print("<CC:");
            out.print(cover.getStatus() == 2 ? "true" : "false");
            out.print(">");
            return true;

        case CCSerialCommand::OpenCover:
            if (!cover.isEnable()) {
                out.print("<CC:ERR:NOT_ENABLE>");
                return false;
            }
            if (cover.canOpen()) {
                cover.open();
                out.print("<CC:OK>");
                return true;
            }
            out.print("<CC:ERR:CAN_T_OPEN>");
            return false;

        case CCSerialCommand::CloseCover:
            if (!cover.isEnable()) {
                out.print("<CC:ERR:NOT_ENABLE>");
                return false;
            }
            if (cover.canClose()) {
                cover.close();
                out.print("<CC:OK>");
                return true;
            }
            out.print("<CC:ERR:CAN_T_CLOSE>");
            return false;

        case CCSerialCommand::HaltCover:
            out.print("<CC:ERR:NOT_IMPL>");
            return true;
    }

    #pragma endregion

    // Se siamo qui, il comando non è stato gestito
    LOGI("Command not handled: %s", cmd);
    out.print("<CC:ERR:BAD_CMD:DRIVER_EXC>");
    return false;
}