#include "CoverCalibratorModule.h"
#include "esp_log.h"
#define LOG_TAG "CovCal"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

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
  if (strcmp(cmd, "DEVICE_STATE") == 0)     return CCSerialCommand::DeviceState;
  if (strcmp(cmd, "CAL_BRI") == 0)     return CCSerialCommand::Brightness;
  if (strcmp(cmd, "CAL_MAX_BRI") == 0)     return CCSerialCommand::MaxBrightness;
  if (strcmp(cmd, "CAL_CHANGING") == 0)     return CCSerialCommand::CalibratorChanging;
  if (strcmp(cmd, "CAL_STATE") == 0)     return CCSerialCommand::CalibratorState;
  if (strcmp(cmd, "CAL_OFF") == 0)     return CCSerialCommand::CalibratorOff;
  if (strcmp(cmd, "CAL_ON") == 0)     return CCSerialCommand::CalibratorOn;
  if (strcmp(cmd, "COV_MOVING") == 0)     return CCSerialCommand::CoverMoving;
  if (strcmp(cmd, "COV_STATE") == 0)     return CCSerialCommand::CoverState;
  if (strcmp(cmd, "COV_OPEN") == 0)     return CCSerialCommand::OpenCover;
  if (strcmp(cmd, "COV_CLOSE") == 0)     return CCSerialCommand::CloseCover;
  if (strcmp(cmd, "COV_HALT") == 0)     return CCSerialCommand::HaltCover;

  LOGI("Command not found: %s",cmd);
  return CCSerialCommand::Unknown;
}

bool CoverCalibratorModule::isCalibratorCommand(CCSerialCommand cmd){
  switch (cmd)
  {
  case CCSerialCommand::Brightness:
  case CCSerialCommand::MaxBrightness:
  case CCSerialCommand::CalibratorChanging:
  case CCSerialCommand::CalibratorOff:
  case CCSerialCommand::CalibratorOn:
  case CCSerialCommand::CalibratorState:
    return true;
  }
  return false;
}

bool CoverCalibratorModule::isCoverCommand(CCSerialCommand cmd){
  switch (cmd)
  {
  case CCSerialCommand::CoverMoving:
  case CCSerialCommand::CoverState:
  case CCSerialCommand::OpenCover:
  case CCSerialCommand::CloseCover:
  case CCSerialCommand::HaltCover:
    return true;
  }
  return false;
}

bool CoverCalibratorModule::handlePacket(char* payload, Stream& out) {
    char* saveptr = nullptr;
    char* cmd = strtok_r(payload, ":", &saveptr);

    if (cmd == nullptr) {
      out.print("<ERR:BAD_CMD:NULLPTR>");
      return false;
    }

    /*
    If module is not enable refuse all commands
    */
/*
    if(!isEnable()){
      out.print("<ERR:NOT_ENABLE>");
      return false;
    }
/*/
    CCSerialCommand command;
    LOGI("Command recived: %s",cmd);
    command = parseCommand(cmd);
    /*
    If command is not listed return the error
    */
    if (command == CCSerialCommand::Unknown) {
      out.print("<ERR:BAD_CMD:UNKNOW>");
      return false;
    }

    switch (command){

      case CCSerialCommand::Desc:
        out.print("<OK:");
        out.print(getIdentifier());
        out.print("- TeslaBoard via USB>");
        out.print(">");
        return true;

      case CCSerialCommand::IntVersion:
        out.print("<");
        out.print("2");
        out.print(">");
        return true;

      case CCSerialCommand::Name:
        out.print("<");
        out.print(getIdentifier());
        out.print("- TeslaBoard>");
        return true;
      case CCSerialCommand::Connect:
      case CCSerialCommand::Disconnect:
        out.print("<OK>");
        return true;

      case CCSerialCommand::Connected:
        out.print("<true>");
        return true;

      case CCSerialCommand::Connecting:
        out.print("<false>");
        return true;
      case CCSerialCommand::SupportedActions:
        out.print("<>");
        return true;

      /* Not Implemented metods/property*/
      case CCSerialCommand::Action:
      case CCSerialCommand::CmdBlind:
      case CCSerialCommand::CmdBool:
      case CCSerialCommand::CmdString:
      case CCSerialCommand::HaltCover:
        out.print("<ERR:NOT_IMPL>");
        return true;

      case CCSerialCommand::DeviceState:
        out.print("<ERR:TO_DO>");
        return true;
    }

    if(isCalibratorCommand(command)){

      if(command == CCSerialCommand::CalibratorState){
        out.print("<");
        out.print(calibrator.getStatus());
        out.print(">");
        return true;
      }

      if(!calibrator.isEnable()){
        out.print("<ERR:NOT_ENABLE>");
        return false;
      }

      char* chBrightness;
      int brightness;
      
      switch (command)
      {
        case CCSerialCommand::Brightness:
          out.print("<");
          out.print(calibrator.getBrightness());
          out.print(">");
          return true;
        case CCSerialCommand::MaxBrightness:
          out.print("<");
          out.print(calibrator.getMaxBrightness());
          out.print(">");
          return true;        
        case CCSerialCommand::CalibratorChanging:
          out.print("<false>");
          return true;
        case CCSerialCommand::CalibratorOff:
          out.print("<OK>");
          calibrator.setBrightness(0);
          return true; 
        case CCSerialCommand::CalibratorOn:
          chBrightness = strtok_r(nullptr, ":", &saveptr);
          LOGI("Brightness stroked: %s", chBrightness);
          brightness = atoi(chBrightness);
          LOGI("Calibrator on request");
          LOGI("Brightness requested: %d", brightness);
          if(brightness < 0 || brightness > calibrator.getMaxBrightness()){
            out.print("<ERR:BRIGHT_OUT_OF_RANGE>");
            return false;
          }
          out.print("<OK>");
          calibrator.setBrightness(brightness);
          return true;
      }
    }

    if(isCoverCommand(command)){

      if(command == CCSerialCommand::CoverState){
        out.print("<");
        out.print(cover.getStatus());
        out.print(">");
        return true;
      }
      if(!cover.isEnable()){
        out.print("<ERR:NOT_ENABLE>");
        return false;
      }

      switch (command)
      {
        case CCSerialCommand::CoverMoving:
          out.print("<");
          if(cover.getStatus() == 2){
            out.print("true");
          } else {
            out.print("false");
          }
          out.print(">");
          return true;

        case CCSerialCommand::OpenCover:
          if(cover.canOpen()){
            out.print("<OK>");
            return true;    
          }
          out.print("<ERR:CAN_T_OPEN>");
          return false;
              
        case CCSerialCommand::CloseCover:
          if(cover.canClose()){
            out.print("<OK>");
            return true;    
          }
          out.print("<ERR:CAN_T_CLOSE>");
          return false;
      }

    }

    /* If i'm here I don't know why :( */
    LOGI("No switch fired?");
    out.print("<ERR:BAD_CMD:DRIVER_EXC>");
    return false;

  }
