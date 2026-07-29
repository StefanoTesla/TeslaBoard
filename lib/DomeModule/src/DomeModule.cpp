#include "DomeModule.h"
#undef LOG_TAG
#define LOG_TAG "Dome"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

#pragma region Configuration
/* here we write additional data if nvs was empty*/
void DomeModule::initSecondaryData() {
    NvsManager::getInstance().putString("shutter", "{}");
}

/* here we load secondary data during the begin */
void DomeModule::loadSecondaryData() {

    String cfg = NvsManager::getInstance().getString("shutter","{}");
    LOGD("raw shutter json is: %s",cfg.c_str());
    DeserializationError error = deserializeJson(tmpCfg, cfg);
    
    LOGD("shutter deserialization ret val: %d 0=no error",error);

    if(!error){
        shutter.begin(tmpCfg);
    }

    if(shutter.isEnable()){
      moduleEnable = true;
    }
    tmpCfg.clear();
}


/* here we update the nvs when new schema is given */
bool DomeModule::applySchemaUpgradeStep(uint16_t currentVersion) {
    LOGI("Applying schema upgrade step from version %u", currentVersion);

    if (!NvsManager::getInstance().openNVS(false, DOME_SCHEMA_NAME)) {
        LOGE("Unable to open board namespace for schema upgrade");
        return false;
    }

    switch (currentVersion) {
        case 0:
            NvsManager::getInstance().putString("identifier", "Dome");
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
void DomeModule::appendSecondaryConfig(JsonObject dest) {
    JsonObject shutterObj = dest["shutter"].to<JsonObject>();
    shutter.getConfiguration(shutterObj);
}


/* here the validation of secondary data when store configuration is called*/
bool DomeModule::validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) {
    JsonArray err = response["errors"].as<JsonArray>();
    if(!toBeValidated["shutter"].is<JsonObject>()){
        LOGE("Shutter Configuration is missing or is not a json object");
        err.add("Shutter Configuration is missing");
        return false;
    }
    
    shutter.validateConfiguration(toBeValidated["shutter"],response);

    rebootNeeded = response["reboot"].as<bool>();
    
    return err.size() == 0;
}

/* here we store secondary data during the save config */
void DomeModule::storeSecondaryConfig(const JsonObject &toBeStored) {
    shutter.storeConfiguration(toBeStored["shutter"],DOME_SCHEMA_NAME);
}



#pragma endregion


bool DomeModule::isEnable(){
    return moduleEnable;
}


void DomeModule::loop(){
    
    if(isEnable()){
        shutter.loop();
    }
}



/* SERIAL MANAGER */

DomeModule::DomeSerialCommand DomeModule::parseCommand(const char* cmd) {
  if (strcmp(cmd, "DEVICE_STATE") == 0)     return DomeSerialCommand::DeviceState;
  if (strcmp(cmd, "OPEN_SH") == 0)     return DomeSerialCommand::OpenShutter;
  if (strcmp(cmd, "CLOSE_SH") == 0)     return DomeSerialCommand::CloseShutter;
  if (strcmp(cmd, "CAN_SET_SH") == 0)     return DomeSerialCommand::CanSetShutter;
  if (strcmp(cmd, "SH_STATUS") == 0)     return DomeSerialCommand::ShutterStatus;
  if (strcmp(cmd, "ABORT") == 0)     return DomeSerialCommand::AbortSlew;

  if (strcmp(cmd, "NAME") == 0)           return DomeSerialCommand::Name;
  if (strcmp(cmd, "DESC") == 0)           return DomeSerialCommand::Desc;
  if (strcmp(cmd, "INT_VRS") == 0)        return DomeSerialCommand::IntVersion;
  if (strcmp(cmd, "SUP_ACTIONS") == 0)    return DomeSerialCommand::SupportedActions;
  if (strcmp(cmd, "ACTION") == 0)         return DomeSerialCommand::Action;
  if (strcmp(cmd, "CMD_BLIND") == 0)      return DomeSerialCommand::CmdBlind;
  if (strcmp(cmd, "CMD_BOOL") == 0)       return DomeSerialCommand::CmdBool;
  if (strcmp(cmd, "CMD_STRING") == 0)     return DomeSerialCommand::CmdString;
  if (strcmp(cmd, "CONNECT") == 0)        return DomeSerialCommand::Connect;
  if (strcmp(cmd, "CONNECTING") == 0)        return DomeSerialCommand::Connecting;
  if (strcmp(cmd, "CONNECTED") == 0)        return DomeSerialCommand::Connected;
  if (strcmp(cmd, "DISCONNECT") == 0)     return DomeSerialCommand::Disconnect;
  if (strcmp(cmd, "SLAVED") == 0)     return DomeSerialCommand::Slaved;
  if (strcmp(cmd, "SLEWING") == 0)     return DomeSerialCommand::Slewing;
  if (strcmp(cmd, "ALTITUDE") == 0)     return DomeSerialCommand::Altitude;
  if (strcmp(cmd, "AZIMUTH") == 0)     return DomeSerialCommand::Azimuth;
  if (strcmp(cmd, "AT_HOME") == 0)     return DomeSerialCommand::AtHome;
  if (strcmp(cmd, "AT_PARK") == 0)     return DomeSerialCommand::AtPark;
  if (strcmp(cmd, "SLEW_ALTI") == 0)     return DomeSerialCommand::SlewToAltitude;
  if (strcmp(cmd, "SLEW_AZI") == 0)     return DomeSerialCommand::SlewToAltitude;
  if (strcmp(cmd, "CAN_HOME") == 0)     return DomeSerialCommand::CanFindHome;
  if (strcmp(cmd, "CAN_SET_AZI") == 0)     return DomeSerialCommand::CanSetAzi;
  if (strcmp(cmd, "CAN_SET_PARK") == 0)     return DomeSerialCommand::CanSetPark;
  if (strcmp(cmd, "CAN_SLAVE") == 0)     return DomeSerialCommand::CanSlave;
  if (strcmp(cmd, "CAN_SYNC_AZI") == 0)     return DomeSerialCommand::CanSyncAzimuth;
  if (strcmp(cmd, "FIND_HOME") == 0)     return DomeSerialCommand::FindHome;
  if (strcmp(cmd, "PARK") == 0)     return DomeSerialCommand::Park;
  if (strcmp(cmd, "SET_PARK") == 0)     return DomeSerialCommand::SetPark;

  return DomeSerialCommand::Unknown;
}

bool DomeModule::handlePacket(char* payload, Stream& out) {
    char* saveptr = nullptr;
    char* cmd = strtok_r(payload, ":", &saveptr);

    if (cmd == nullptr) {
      out.print("<ERR:BAD_CMD:NULLPTR>");
      return false;
    }

    /*
    If module is not enable refuse all commands
    */

    if(!isEnable()){
      out.print("<ERR:NOT_ENABLE>");
      return false;
    }

    DomeSerialCommand command;
    LOGI("Command recived: %d",cmd);
    command = parseCommand(cmd);
    /*
    If command is not listed return the error
    */
    if (command == DomeSerialCommand::Unknown) {
      out.print("<ERR:BAD_CMD:UNKNOW>");
      return false;
    }
    
    switch (command){

      case DomeSerialCommand::Name:
        out.print("<");
        out.print(getIdentifier());
        out.print("- TeslaBoard>");
        return true;

      case DomeSerialCommand::Desc:
        out.print("<OK:");
        out.print("Dome - TeslaBoard 4.0 via USB");
        out.print(">");
        return true;

      case DomeSerialCommand::IntVersion:
        out.print("<");
        out.print("3");
        out.print(">");
        return true;

      case DomeSerialCommand::Connect:
      case DomeSerialCommand::Disconnect:
        out.print("<OK>");
        return true;

      case DomeSerialCommand::Connecting:
        out.print("<false>");
        return true;
      case DomeSerialCommand::Connected:
        out.print("<true>");
        return true;

      case DomeSerialCommand::SupportedActions:
        out.print("<>");
        return true;

      /* Not Implemented metods/property*/
      case DomeSerialCommand::Action:
      case DomeSerialCommand::CmdBlind:
      case DomeSerialCommand::CmdBool:
      case DomeSerialCommand::CmdString:
      case DomeSerialCommand::Slaved:
      case DomeSerialCommand::AtHome:
      case DomeSerialCommand::AtPark:
      case DomeSerialCommand::Azimuth:
      case DomeSerialCommand::Altitude:
      case DomeSerialCommand::SlewToAltitude:
      case DomeSerialCommand::FindHome:
      case DomeSerialCommand::Park:
      case DomeSerialCommand::SetPark:
      case DomeSerialCommand::SlewToAzimuth:
        out.print("<ERR:NOT_IMPL>");
        return true;

      /* thing this board can't do*/
      case DomeSerialCommand::CanFindHome:
      case DomeSerialCommand::CanSetAzi:
      case DomeSerialCommand::CanSetPark:
      case DomeSerialCommand::CanSlave:
      case DomeSerialCommand::CanSyncAzimuth:
        out.print("<false>");
        return true;
      /* thing this board can do*/ 
      case DomeSerialCommand::CanSetShutter:
        out.print("<false>");
        return true;

      case DomeSerialCommand::ShutterStatus:
        out.print("<");
        out.print(shutter.getStatus());
        out.print(">");
        return true;

      case DomeSerialCommand::AbortSlew:
        shutter.halt();
        out.print("<false>");
        return true;

      case DomeSerialCommand::OpenShutter:
        if(shutter.canOpen()){
          shutter.open();
          out.print("<OK>");
          return true;
        }

        if(shutter.getActualCommand() != 0 ){
          out.print("<ERR:ALREADY_MOVING>");
          return false;
        } else {
          out.print("<ERR:ALREADY_OPEN>");
          return false;
        }
        
      case DomeSerialCommand::CloseShutter:
        if(shutter.canClose()){
          shutter.close();
          out.print("<OK>");
          return true;
        }

        if(shutter.getActualCommand() != 0 ){
          out.print("<ERR:ALREADY_MOVING>");
          return false;
        } else {
          out.print("<ERR:ALREADY_CLOSED>");
          return false;
        }

      case DomeSerialCommand::Slewing:
        if(shutter.getActualCommand() != 0){
          out.print("<true>");
          return true;
        } else {
          out.print("<false>");
          return true;
        }

    }
   
    #pragma endregion


    /* If i'm here I don't know why :( */
    out.print("<ERR:BAD_CMD:UNKNOW_CMD>");
    return false;
  }
