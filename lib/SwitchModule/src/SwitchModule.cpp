#include "SwitchModule.h"
#undef LOG_TAG
#define LOG_TAG "Switch"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)


#pragma region Configuration
/* here we write additional data if nvs was empty*/

/* here we write additional data if nvs was empty*/
void SwitchModule::initSecondaryData() {
  NvsManager::getInstance().putInt("cfg_sw", 0);
  for (size_t i = 0; i < SWITCH_MAX_SWITCHES; i++) {
    char key[10];
    sprintf(key, "sw%d", i);
    NvsManager::getInstance().removeKey(key);
  }
  NvsManager::getInstance().putInt("schema", 1);
}

/* here we load secondary data during the begin */
void SwitchModule::loadSecondaryData() {

  configuredSwitches = NvsManager::getInstance().getInt("cfg_sw", 0);

    if (configuredSwitches == 0) {
      LOGI("Any switches configured yet, disabling the module...");
      moduleEnable = false;
      return;
  }

  int id = -1;
  
  for (size_t i = 0; i < configuredSwitches; i++) {
    id++;
    tmpCfg.clear();
    // prepare the page key "swX"
    char key[10];
    sprintf(key, "sw%d", i);
    // get the string stored
    String swString;
    swString = NvsManager::getInstance().getString(key, "{\"type\":0}");
    DeserializationError err;
    err = deserializeJson(tmpCfg, swString);

    if (err != DeserializationError::Ok) {
      LOGE("Error during switch id: %d deserializzation");
      continue;
    }

    JsonObjectConst tmp = tmpCfg.as<JsonObjectConst>();

    Type type;
    type = static_cast<Type>(tmp["type"].as<int>());

    if (type == Type::NotPresent) {
      continue;
    } else if (type == Type::Input) {
      Switches[i] = new DigitalInput;
      Switches[i]->jsonSetup(tmp);
    } else if (type == Type::Output) {
      Switches[i] = new DigitalOutput;
      Switches[i]->jsonSetup(tmp);
    } else if (type == Type::PWM) {
      Switches[i] = new PWMOutput(chMgr);
      if (!Switches[i]->jsonSetup(tmp)) {
        LOGE("unable to setup PWM Output");
        Switches[i] = nullptr;
      };
    } else if (type == Type::Servo) {
      Switches[i] = new ServoOutput(chMgr);
      if (!Switches[i]->jsonSetup(tmp)) {
        LOGE("unable to setup Servo Output");
        Switches[i] = nullptr;
      };
    } else {
      LOGE("Wrong Type stored on NVS");
    }
  }

  configuredSwitches = id + 1;

  LOGV("Switch begin finished");
  LOGV("%d confugred switches", configuredSwitches);
}

/* here we update the nvs when new schema is given */
bool SwitchModule::applySchemaUpgradeStep(uint16_t currentVersion) {
    LOGI("Applying schema upgrade step from version %u", currentVersion);

    if (!NvsManager::getInstance().openNVS(false, SWITCH_SCHEMA_NAME)) {
        LOGE("Unable to open board namespace for schema upgrade");
        return false;
    }

    switch (currentVersion) {
        case 0:
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
void SwitchModule::appendSecondaryConfig(JsonObject dest) {
  JsonArray switchesArray = dest["Switches"].to<JsonArray>();
    for (size_t i = 0; i < configuredSwitches; i++) {
    if (Switches[i] == nullptr || Switches[i]->getType() > Type::Servo) {
      continue;
    }

    JsonObject swi = switchesArray.add<JsonObject>();
    Switches[i]->getConfiguration(swi);
  }
}

/* here the validation of secondary data when store configuration is called*/
bool SwitchModule::validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) {
    JsonArray err = response["errors"].as<JsonArray>();

    if (!toBeValidated["Switches"].is<JsonArray>()) {
        LOGE("Switches is not an array");
        err.add("Switches is not an array");
        return false;
    }

    JsonArray switches = toBeValidated["Switches"].as<JsonArray>();
    JsonArray incomingSwitches = tmpCfg["Switches"].to<JsonArray>();

    int id = -1;

    for (JsonObject singleSW : switches) {
        if (!singleSW["type"].is<unsigned int>()) {
            LOGE("Type is missing or not an unsigned int");
            err.add("Switch type is missing or invalid");
            return false;
        }

        unsigned int typeValue = singleSW["type"].as<unsigned int>();

        if (typeValue > Type::Servo) {
            LOGE("Type value %u is out of range", typeValue);
            JsonObject e = err.add<JsonObject>();
            e["error"] = "Type out of range";
            return false;
        }

        Type type = static_cast<Type>(typeValue);

        if (type == Type::NotPresent) {
            LOGV("Type is NotPresent, skipping");
            continue;
        }

        id++;

        int retVal = validateSwitchType(type, singleSW);
        if (retVal != 1) {
            LOGE("Validation failed for switch id: %d with error code: %d", id, retVal);
            JsonObject e = err.add<JsonObject>();
            e["id"] = id;
            e["error"] = retVal;
            return false;
        }

        incomingSwitches.add(singleSW);
        checkIfRebootNeeded(id, type, singleSW, response);
    }

    if (incomingSwitches.size() != configuredSwitches) {
        response["reboot"] = true;
    }

    rebootNeeded = response["reboot"].as<bool>();
    LOGI("All switches validated successfully");

    return err.size() == 0;
}

void SwitchModule::storeSecondaryConfig(const JsonObject &toBeStored) {
  int id = -1;

  JsonDocument sanDoc;
  JsonObject sanitizedObject = sanDoc.to<JsonObject>();

  serializeJson(tmpCfg["Switches"].as<JsonArray>(), Serial);
  for (JsonObject inSwitch : tmpCfg["Switches"].as<JsonArray>()) {
    sanitizedObject.clear();

    if (!inSwitch["type"].is<int>()) {
      LOGE("This should never happens! missing type in storing configuration");
      continue;
    }
    Type type = static_cast<Type>(inSwitch["type"].as<unsigned int>());
    if (NotPresent == type) {
      LOGE("This should never happens! NotPresent type in storing "
           "configuration");
      continue;
    }

    id++;

    // sanitize the json arriving from the web
    switch (type) {
    case Input:
      DigitalInput::copyJsonCfg(inSwitch, sanitizedObject);
      break;
    case Output:
      DigitalOutput::copyJsonCfg(inSwitch, sanitizedObject);
      break;
    case PWM:
      PWMOutput::copyJsonCfg(inSwitch, sanitizedObject);
      break;
    case Servo:
      ServoOutput::copyJsonCfg(inSwitch, sanitizedObject);
      break;

    default:
      LOGE("This should never happens! Undefined type during sanitifaction");
      break;
    }

    // apply soft parameters
    // if already configured is the same type and same pin
    if (Switches[id] != nullptr) {
      if (Switches[id]->getType() ==
              sanitizedObject["type"].as<unsigned int>() &&
          Switches[id]->getPinNumber() ==
              sanitizedObject["pin"].as<unsigned int>()) {

        Switches[id]->setName(sanitizedObject["name"].as<const char *>());
        Switches[id]->setDescription(
            sanitizedObject["desc"].as<const char *>());

        switch (type) {
        case Input:
          Switches[id]->setDelays(sanitizedObject["dOn"].as<unsigned int>(),
                                  sanitizedObject["dOff"].as<unsigned int>());
          Switches[id]->setInvert(sanitizedObject["invert"].as<bool>());
          break;
        case Output:
          Switches[id]->setInvert(sanitizedObject["invert"].as<bool>());
          break;
        case PWM:
          // nothing to do here
          break;
        case Servo:
          Switches[id]->setMoveTime(
              sanitizedObject["moveTime"].as<unsigned int>());
          break;

        default:
          LOGE(
              "This should never happens! Undefined type during sanitifaction");
          break;
        }
      }
    }

    char key[10];
    sprintf(key, "sw%d", id);
    String swString;
    serializeJson(sanitizedObject, swString);
    NvsManager::getInstance().putString(key, swString);
  }

  NvsManager::getInstance().putInt("cfg_sw", id + 1);

}


/* helpers */
int SwitchModule::validateSwitchType(Type type, const JsonObject &singleSW) {
  switch (type) {
  case NotPresent:
    return 1;
  case Input:
    return DigitalInput::validateJsonCfg(singleSW);
  case Output:
    return DigitalOutput::validateJsonCfg(singleSW);
  case PWM:
    return PWMOutput::validateJsonCfg(singleSW);
  case Servo:
    return ServoOutput::validateJsonCfg(singleSW);
  default:
    LOGV("Undefined type");
    return 0;
  }
}

void SwitchModule::checkIfRebootNeeded(int id, Type type, const JsonObject &singleSW, JsonObject response) {

  if (Switches[id] == nullptr) {
    LOGV("Switch %d: new position, reboot needed", id);
    response["reboot"] = true;
    return;
  }

  if (Switches[id]->getType() != static_cast<unsigned int>(type)) {
    LOGV("Switch %d: type changed, reboot needed", id);
    response["reboot"] = true;
    return;
  }

  if (Switches[id]->getPinNumber() != singleSW["pin"].as<unsigned int>()) {
    LOGV("Switch %d: pin changed, reboot needed", id);
    response["reboot"] = true;
    return;
  }

  LOGV("Switch %d: no changes detected", id);
}

#pragma endregion

void SwitchModule::loop() {

  for (size_t i = 0; i < configuredSwitches; i++) {
    if (Switches[i] == nullptr) {
      continue;
    }

    if (Switches[i]->getType() == Type::Input || Switches[i]->getType() == Type::Servo) {
      Switches[i]->loop();
    }
  }
}

void SwitchModule::reportSwitchState(int id, JsonObject status) {

  if (Switches[id] == nullptr) {
    return;
  }
  status["type"] = Switches[id]->getType();
  status["name"] = Switches[id]->getName();
  status["desc"] = Switches[id]->getDescription();
  status["min"] = Switches[id]->getMin();
  status["max"] = Switches[id]->getMax();
  if (Switches[id]->getType() <= 2) {
    status["status"] = (bool)Switches[id]->status();
  } else {
    status["status"] = Switches[id]->status();
  }
}


/*
Check if provided id is valid
1 = OK
-1 = ID outside limits
-2 = unconfigured Switch
*/
int SwitchModule::isValidID(int id){

  if(id<0 || id>= configuredSwitches){
    return -1;
  }

  if(Switches[id] == nullptr){
    return -2;
  }

  return 1;
}


/*
Check if provided id is writable
1 = OK
-1 = ID outside limits
-2 = unconfigured Switch
-3 = unwritable switch
*/
int SwitchModule::isWritable(int id){

  int validID = isValidID(id);

  if(validID != 1){ return validID;}

  if(Switches[id]->getType() == Type::Input){
    return -3;
  }

  return 1;
}
/*
Check if provided id is valid
1 = OK
-1 = ID outside limits
-2 = unconfigured Switch
-4 = value lower than min value
-5 = value greater than max value
*/
int SwitchModule::isValidValue(int id, int value){

  int validID=isValidID(id);
  if(validID!=1){ return validID; }

  int min = Switches[id]->getMin();
  int max = Switches[id]->getMax();

  if(value < min){ return -4; }
  if(value > max){ return -5; }

  return 1;
}


/*
To be used only for servo
1 = OK
-1 = ID outside limits
-2 = unconfigured Switch
-3 = unwritable Switch
-4 = value lower than min value
-5 = value greater than max value
-6 = is not a servo
*/
int SwitchModule::setServoPositionAsync(int id, int position){

  int validID=isValidID(id);
  if(validID!=1){ return validID; }

  if(!isWritable(id)){ return -3;}

  int validPos = isValidValue(id,position);
  if(validPos != 1){ return validPos; }
  
  if (Switches[id]->getType() == Type::Servo) {
      ServoOutput *servo = static_cast<ServoOutput *>(Switches[id]);
      servo->goTo(position, false, false);
      return 1;
  }
  
  return -6;

}

/*
Write a value on the Switch

Return:

1 = OK
-1 = ID outside limits
-2 = unconfigured Switch
-3 = unwritable Switch
-4 = value lower than min value
-5 = value greater than max value

*/
int SwitchModule::setSwitchValue(int id, int value) {

  int validID = isValidID(id);
  if(validID != 1){ return validID; }

  if (!isWritable(id)) { return -3;}

  int validState = isValidValue(id,value);
  if(validState != 1){ return validState;}

  

  if (Switches[id]->getType() == Type::Servo) {
    ServoOutput *servo = static_cast<ServoOutput *>(Switches[id]);
    servo->goTo(value, false, true);
    return 1;
  } else {
    LOGV("Writing switch id:%d with value %d",id,value);
    Switches[id]->write(value);
    return 1;
  }


  return false;
}

int SwitchModule::getSwitchState(int id) {

  if (id >= configuredSwitches) {
    LOGE("Accessing outside the array");
    return -1;
  }
  if (Switches[id] == nullptr) {
    LOGE("Accessing to a nullptr, id: %d", id);
    return -1;
  }

  return Switches[id]->status();
}

int SwitchModule::getType(int id) { return Switches[id]->getType(); }
int SwitchModule::getMax(int id) { return Switches[id]->getMax(); }
int SwitchModule::getMin(int id) { return Switches[id]->getMin(); }

const char* SwitchModule::getSwitchName(int id){

  int retVal = isValidID(id);
  if(isValidID(id) == 1){
      return Switches[id]->getName();
  }

  return "notExist";
}

const char* SwitchModule::getSwitchDescription(int id){

  int retVal = isValidID(id);
  if(isValidID(id) == 1){
      return Switches[id]->getDescription();
  }

  return "notExist";
}

/*
Check if provided id is valid
1 = Is Moving
0 = Is not moving
-1 = ID outside limits
-2 = unconfigured Switch
*/
int SwitchModule::getServoIsMoving(int id){

  int validID = isValidID(id);
  if(validID != 1){
    return validID;
  }

  if(getType(id) == Type::Servo){
    ServoOutput *servo = static_cast<ServoOutput *>(Switches[id]);
    if(servo->isMoving()){
      return 1;
    }
  } 
  return 0;
}

#pragma region Serial

/* SERIAL MANAGER */
SwitchModule::SwitchSerialCommand SwitchModule::parseCommand(const char* cmd) {
  if (strcmp(cmd, "MAX_SWITCH") == 0)      return SwitchSerialCommand::MaxSwitch;
  if (strcmp(cmd, "DESC") == 0)           return SwitchSerialCommand::Desc;
  if (strcmp(cmd, "INT_VRS") == 0)        return SwitchSerialCommand::IntVersion;
  if (strcmp(cmd, "NAME") == 0)           return SwitchSerialCommand::Name;
  if (strcmp(cmd, "SUP_ACTIONS") == 0)    return SwitchSerialCommand::SupportedActions;
  if (strcmp(cmd, "ACTION") == 0)         return SwitchSerialCommand::Action;
  if (strcmp(cmd, "CAN_ASYNC") == 0)      return SwitchSerialCommand::CanAsync;
  if (strcmp(cmd, "CANC_ASYNC") == 0)     return SwitchSerialCommand::CancelAsync;
  if (strcmp(cmd, "CAN_WRITE") == 0)      return SwitchSerialCommand::CanWrite;
  if (strcmp(cmd, "CMD_BLIND") == 0)      return SwitchSerialCommand::CmdBlind;
  if (strcmp(cmd, "CMD_BOOL") == 0)       return SwitchSerialCommand::CmdBool;
  if (strcmp(cmd, "CMD_STRING") == 0)     return SwitchSerialCommand::CmdString;
  if (strcmp(cmd, "CONNECT") == 0)        return SwitchSerialCommand::Connect;
  if (strcmp(cmd, "CONNECTING") == 0)        return SwitchSerialCommand::Connecting;
  if (strcmp(cmd, "DISCONNECT") == 0)     return SwitchSerialCommand::Disconnect;
  if (strcmp(cmd, "CONNECTED") == 0)     return SwitchSerialCommand::Connected;
  if (strcmp(cmd, "GET_SWITCH_VALUE") == 0) return SwitchSerialCommand::GetSwitchValue;
  if (strcmp(cmd, "GET_SWITCH_DESC") == 0)  return SwitchSerialCommand::GetSwitchDesc;
  if (strcmp(cmd, "GET_SWITCH_NAME") == 0)  return SwitchSerialCommand::GetSwitchName;
  if (strcmp(cmd, "GET_SWITCH_MAX") == 0)   return SwitchSerialCommand::GetSwitchMax;
  if (strcmp(cmd, "GET_SWITCH_MIN") == 0)   return SwitchSerialCommand::GetSwitchMin;
  if (strcmp(cmd, "SET_ASYNC") == 0)      return SwitchSerialCommand::SetAsync;
  if (strcmp(cmd, "SET_ASYNC_VAL") == 0)  return SwitchSerialCommand::SetAsyncVal;
  if (strcmp(cmd, "STATE_CHANGE_OK") == 0)  return SwitchSerialCommand::StateChangeComplete;
  if (strcmp(cmd, "SET") == 0)            return SwitchSerialCommand::Set;
  if (strcmp(cmd, "SET_NAME") == 0)       return SwitchSerialCommand::SetName;
  if (strcmp(cmd, "SET_DESC") == 0)       return SwitchSerialCommand::SetDesc;
  if (strcmp(cmd, "SET_VALUE") == 0)      return SwitchSerialCommand::SetValue;
  if (strcmp(cmd, "STEP") == 0)           return SwitchSerialCommand::Step;
  if (strcmp(cmd, "DEVICE_STATE") == 0)     return SwitchSerialCommand::DeviceState;

  return SwitchSerialCommand::Unknown;
}

bool SwitchModule::handlePacket(char* payload, Stream& out) {
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

    SwitchSerialCommand command;
    LOGI("Command received: %s", cmd ? cmd : "(null)");
    command = parseCommand(cmd);
    /*
    If command is not listed return the error
    */
    if (command == SwitchSerialCommand::Unknown) {
      out.print("<ERR:BAD_CMD:UNKNOW>");
      return false;
    }

    /* command that don't require the ID Valitation*/
    #pragma region CmdWithoutIDValidation 
    
    switch (command){

      case SwitchSerialCommand::MaxSwitch:
        out.print("<");
        out.print(getConfiguredSwitch());
        out.print(">");
        return true;

      case SwitchSerialCommand::Desc:
        out.print("<OK:");
        out.print("Switch - TeslaBoard 4.0");
        out.print(">");
        return true;

      case SwitchSerialCommand::IntVersion:
        out.print("<");
        out.print("3");
        out.print(">");
        return true;

      case SwitchSerialCommand::Name:
        out.print("<");
        out.print(getIdentifier());
        out.print("- TeslaBoard>");
        return true;
      case SwitchSerialCommand::Connect:
      case SwitchSerialCommand::Disconnect:
        out.print("<OK>");
        return true;

      case SwitchSerialCommand::Connected:
        out.print("<true>");
        return true;

      case SwitchSerialCommand::Connecting:
        out.print("<false>");
        return true;
      case SwitchSerialCommand::SupportedActions:
        out.print("<>");
        return true;

      /* Not Implemented metods/property*/
      case SwitchSerialCommand::Action:
      case SwitchSerialCommand::CmdBlind:
      case SwitchSerialCommand::CmdBool:
      case SwitchSerialCommand::CmdString:
      case SwitchSerialCommand::SetName:
      case SwitchSerialCommand::SetDesc:
        out.print("<ERR:NOT_IMPL>");
        return true;

      case SwitchSerialCommand::DeviceState:
        out.print("<ERR:TO_DO>");
        return true;
    }
   
    #pragma endregion

    /* get the switch id */
    char* chStr = strtok_r(nullptr, ":", &saveptr);

      if (chStr == nullptr || *chStr == '\0') {
          out.print("<ERR:BAD_CMD:INVALID_ID>");
          return false;
      }

      /* convert with error checking */
      char* endPtr = nullptr;
      long val = strtol(chStr, &endPtr, 10);

      /* must be a pure integer, no trailing chars */
      if (*endPtr != '\0') {
          out.print("<ERR:BAD_CMD:INVALID_ID>");
          return false;
      }

      /* id is 1-based in the protocol, array is 0-based */
      if (val < 1 || val > getConfiguredSwitch()) {  
          out.print("<ERR:BAD_CMD:ID_RANGE>");
          return false;
      }

      const int id = static_cast<int>(val) - 1;

    if(isValidID(id) != 1){
      out.print("<ERR:INVALID_ID>");
      return false;
    }

    #pragma region IDValidation

    switch (command){

      case  SwitchSerialCommand::CanAsync:
      case  SwitchSerialCommand::CancelAsync:
        out.print("<OK>");
        return true;

      case SwitchSerialCommand::GetSwitchName:
        out.print("<");
        out.print(Switches[id]->getName());
        out.print(">");
        return true;

      case SwitchSerialCommand::GetSwitchDesc:
        out.print("<");
        out.print(Switches[id]->getDescription());
        out.print(">");
        return true;

      case SwitchSerialCommand::GetSwitchMin:
        out.print("<");
        out.print(Switches[id]->getMin());
        out.print(">");
        return true;

      case SwitchSerialCommand::GetSwitchMax:
        out.print("<");
        out.print(Switches[id]->getMax());
        out.print(">");
        return true;

      case SwitchSerialCommand::Step:
        out.print("<1>");
        return true;

      case SwitchSerialCommand::CanWrite:
        if(isWritable(id) == 1){
          out.print("<TRUE>");
        } else {
          out.print("<FALSE>");
        }
        return true;
      case SwitchSerialCommand::GetSwitchValue:
        out.print("<");
        out.print(Switches[id]->status());
        out.print(">");
        return true;
      case SwitchSerialCommand::StateChangeComplete:

        if(Switches[id]->getType() == Type::Servo){
          if(getServoIsMoving(id) == 1){
            out.print("<false>");
            return true;
          } 
          out.print("<true>");
        }
        return true;
            
    }

    #pragma endregion

    /*
    from here only command whre the switch need to be writable
    */

    if(isWritable(id) != 1){
      out.print("<ERR:ID_NOT_WRITABLE>");
      return false;
    }

    char* stateStr =nullptr;
    char* valueStr =nullptr;
    bool State;
    int Value;
    /* state commands min or max*/
    #pragma region StateValidation 

    switch (command){

      case SwitchSerialCommand::SetAsync:
      case SwitchSerialCommand::Set:
        stateStr = strtok_r(nullptr, ":", &saveptr);
        if (stateStr == nullptr) {
          out.print("<ERR:BAD_CMD:STATE_NULLPTR>");
          return false;
        }
        State = false;

        if( 
          strcmp(stateStr,"True")==0 ||
          strcmp(stateStr,"TRUE")==0 ||
          strcmp(stateStr,"true")==0
        ){
          State= true;
        } else if(
          strcmp(stateStr,"False")==0 ||
          strcmp(stateStr,"FALSE")==0 ||
          strcmp(stateStr,"false")==0
        ){
          State = false;
        } else {
          out.print("<ERR:BAD_CMD:STATE_MALFORMED>");
          return false;
        }

        Switches[id]->write(State ? Switches[id]->getMax() : Switches[id]->getMin());
        out.print("<OK>");
        return true;

    }

    #pragma endregion


    #pragma region ValueValidation 

    switch (command){

      case SwitchSerialCommand::SetAsyncVal:
      case SwitchSerialCommand::SetValue:
        valueStr = strtok_r(nullptr, ":", &saveptr);
        if (valueStr == nullptr || *valueStr == '\0') {
          out.print("<ERR:BAD_CMD:VALUE_NULLPTR>");
          return false;
        }

        {
          char* endPtr = nullptr;
          long val = strtol(valueStr, &endPtr, 10);

          if (*endPtr != '\0') {
            out.print("<ERR:BAD_CMD:VALUE_MALFORMED>");
            return false;
          }

          // opzionale: controllo overflow rispetto a int
          if (val < INT_MIN || val > INT_MAX) {
            out.print("<ERR:BAD_CMD:VALUE_RANGE>");
            return false;
          }

          Value = static_cast<int>(val);
        }

        if (Value < Switches[id]->getMin()) {
          out.print("<ERR:VALUE_UNDER_MIN>");
          return false;
        }
        if (Value > Switches[id]->getMax()) {
          out.print("<ERR:VALUE_ABOVE_MAX>");
          return false;
        }

        Switches[id]->write(Value);
        out.print("<OK>");
        return true;
      }

      #pragma endregion

    /* If i'm here I don't know why :( */
    out.print("<ERR:BAD_CMD:UNKNOW_CMD>");
    return false;
  }

  #pragma endregion