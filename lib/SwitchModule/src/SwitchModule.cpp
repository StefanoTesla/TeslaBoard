#include "SwitchModule.h"
#undef LOG_TAG
#define LOG_TAG "Switch"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

#pragma region nvsHandler

bool SwitchModule::openNVS(bool readOnly) {

  switch (nvsStatus) {

    // nvs is closed, i need to ope according by the readOnly
  case CLOSED:
    LOGV("NVS seems to be closed");
    if (nvs.begin(SWITCH_SCHEMA_NAME, readOnly)) {
      if (readOnly) {
        LOGV("NVS opened in readonly");
        nvsStatus = OPEN_READOLNY;
      } else {
        LOGV("NVS opened with write rights");
        nvsStatus = OPEN_WRITE;
      }
      return true;
    } else {
      LOGE("Error opening the NVS");
      nvsStatus = CLOSED;
      return false;
    }
    break;

  case OPEN_READOLNY:
    LOGV("NVS seems open in read only");
    if (!readOnly) {
      closeNVS();
      if (nvs.begin(SWITCH_SCHEMA_NAME, false)) {
        nvsStatus = OPEN_WRITE;
        LOGV("NVS opened with write rights");
        return true;
      } else {
        LOGV("Error during opening NVS with write rights");
        return false;
      }
    } else {
      LOGV("NVS already open in read only");
      return true;
    }
    break;

  case OPEN_WRITE:
    LOGV("NVS seems open with write rights");
    if (readOnly) {
      closeNVS();
      nvsStatus = CLOSED;
      if (nvs.begin(SWITCH_SCHEMA_NAME, true)) {
        nvsStatus = OPEN_READOLNY;
        LOGV("NVS opened in read only");
        return true;
      } else {
        LOGV("Error during opening NVS in read only");
        return false;
      }
    } else {
      LOGV("NVS already open with write rigts");
      return true;
    }
    break;

  default:
    LOGE("Unknown NVS status: %d", nvsStatus);
    return false;
    break;
  }

  LOGE("Arrived at the buttom of the function, don't know what happed..");
  return false;
}

void SwitchModule::closeNVS() {
  if (nvsStatus != CLOSED) {
    nvs.end();
    nvsStatus = CLOSED;
    LOGV("NVS closed");
  } else {
    LOGV("NVS already closed");
  }
}

#pragma endregion

#pragma region Configuration
/* initialize the switches */
void SwitchModule::begin() {
  LOGI("Loading configuration");
  JsonDocument doc;

  if (!openNVS(true)) {
    LOGE(
        "Error loading switch nvs partition in read only, trying to format it");
    if (!initNVS()) {
      LOGE("NVS INITIALIZATION FAILED");
      moduleEnable = false;
    }
  }

  LOGV("Checking the schema version");
  // if I'm here NVS is surelly working, no more check...for the moment
  openNVS(true);
  int schemaVersion = nvs.getInt("schema", 0);
  LOGD("schema version is: %d", schemaVersion);

  if (schemaVersion < SWITCH_SCHEMA_VERSION) {
    LOGW("Schema version: %d, new version: %d", schemaVersion,
         SWITCH_SCHEMA_VERSION);
    switch (schemaVersion) {
    case 0:
      LOGI("upgrading from 0 to 1");
      updateNVS1();
      break;

    default:
      break;
    }
  }

  openNVS(true);

  moduleEnable = nvs.getBool("enable", false);
  uiOrder = nvs.getInt("order", 1);
  identifier = nvs.getString("identifier", "Switch");

  if (!moduleEnable) {
    LOGW("Module not enable, setup completed");
    closeNVS();
    return;
  }

  configuredSwitches = nvs.getInt("cfg_sw", 0);

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
    swString = nvs.getString(key, "{\"type\":0}");
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
  closeNVS();

  configuredSwitches = id + 1;

  LOGV("Switch begin finished");
  LOGV("%d confugred switches", configuredSwitches);
}

bool SwitchModule::initNVS() {

  LOGW("Switch nvs area will be formatted");
  if (!openNVS(false)) {
    LOGE("Unable to open the namespace with write rights, initialization "
         "failed");
    return false;
  }
  LOGI("namespace open or created, writing default parameters");

  nvs.putBool("enable", false);
  nvs.putInt("schema", 0);
  closeNVS();
  return true;
}

void SwitchModule::updateNVS1() {
  // this is the first schema, don't check if something already exist.
  openNVS(false);
  nvs.putInt("schema", 1);
  nvs.putString("identifier", "Switch");
  nvs.putInt("order", 1);
  nvs.putInt("cfg_sw", 0);

  for (size_t i = 0; i < SWITCH_MAX_SWITCHES; i++) {
    char key[10];
    sprintf(key, "sw%d", i);
    nvs.putString(key, "{\"type\":0}");
  }

  closeNVS();
}

void SwitchModule::getConfiguration(JsonObject dest) {

  dest["enable"] = moduleEnable;
  dest["uiOrder"] = uiOrder;
  dest["identifier"] = identifier;

  JsonArray switchesArray = dest["Switches"].to<JsonArray>();

  for (size_t i = 0; i < configuredSwitches; i++) {
    if (Switches[i] == nullptr || Switches[i]->getType() > Type::Servo) {
      continue;
    }

    JsonObject swi = switchesArray.add<JsonObject>();
    Switches[i]->getConfiguration(swi);
  }
}

void SwitchModule::validateConfiguration(const JsonObject &toBeValidated,
                                         JsonObject response) {
  LOGI("swtich data validation");
  tmpCfg.clear();
  response["reboot"] = false;
  JsonArray err = response["errors"].to<JsonArray>();

  // Validazione campi principali
  if (!validateMainFields(toBeValidated, response)) {
    return;
  }

  if (!toBeValidated["enable"].as<bool>()) {
    LOGI("Main module is not enable, stop validation");
    return;
  }

  LOGI("Main data validation ok, starting with switches data");

  if (!toBeValidated["Switches"].is<JsonArray>()) {
    response["errors"].add("Switches is not an array");
    return;
  }

  validateSwitches(toBeValidated["Switches"].as<JsonArray>(), response);
}

bool SwitchModule::validateMainFields(const JsonObject &data,
                                      JsonObject response) {
  JsonArray err = response["errors"].to<JsonArray>();

  if (!data["enable"].is<bool>()) {
    LOGE("enable don't exist or is not a boolean");
    err.add("Enable is not a boolean");
    return false;
  } else if (moduleEnable != data["enable"]) {
    LOGW("enable changed, reboot requested");
    response["reboot"] = true;
  }

  if (!data["uiOrder"].is<int>()) {
    LOGE("order for ui don't exist or is not a integer");
    err.add("Order is not a number");
    return false;
  }

  if (!data["identifier"].is<String>()) {
    LOGE("board identifier don't exist or is not a string");
    err.add("Identifier is not a string");
    return false;
  }

  tmpCfg["enable"] = data["enable"];
  tmpCfg["uiOrder"] = data["uiOrder"];
  tmpCfg["identifier"] = data["identifier"];

  return true;
}

void SwitchModule::validateSwitches(const JsonArray &switches,
                                    JsonObject response) {

  JsonArray incomingSwitches = tmpCfg["Switches"].to<JsonArray>();
  JsonArray err = response["errors"].to<JsonArray>();
  int id = -1;

  for (JsonObject singleSW : switches) {

    // Controlla se il tipo esiste ed è valido - FAIL FAST
    if (!singleSW["type"].is<unsigned int>()) {
      LOGE("Type is missing or not an unsigned int");
      err.add("Switch type is missing or invalid");
      return;
    }

    unsigned int typeValue = singleSW["type"].as<unsigned int>();

    if (typeValue > 4) {
      LOGE("Type value %u is out of range (max 4)", typeValue);
      JsonObject e = err.add<JsonObject>();
      e["error"] = "Type out of range";
      return;
    }

    Type type = static_cast<Type>(typeValue);

    if (NotPresent == type) {
      LOGV("Type is NotPresent, skipping");
      continue;
    }

    id++;

    int retVal = validateSwitchType(type, singleSW);

    if (retVal != 1) {
      LOGE("Validation failed for switch id: %d with error code: %d", id,
           retVal);
      JsonObject e = err.add<JsonObject>();
      e["id"] = id;
      e["error"] = retVal;
      return;
    }
    incomingSwitches.add(singleSW);
    checkIfRebootNeeded(id, type, singleSW, response);
  }
  if (response["reboot"]) {
    rebootNeeded = true;
  }
  LOGI("All switches validated successfully");
}

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

void SwitchModule::checkIfRebootNeeded(int id, Type type,
                                       const JsonObject &singleSW,
                                       JsonObject response) {

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

void SwitchModule::storeConfiguration() {
  LOGI("Writing new configuration on the NVS");

  serializeJson(tmpCfg, Serial);

  if (!openNVS(false)) {
    LOGE("Failed to open NVS for writing");
    return;
  }

  storeMainFields();

  /* if module is not enable don't write anymore*/
  if (!tmpCfg["enable"].as<bool>()) {
    LOGI("Main Module is not enable, writing new configuration done.");
    closeNVS();
    return;
  }
  LOGI("Main config done, start with switches");

  storeSwitches();

  tmpCfg.clear();
  closeNVS();
}

void SwitchModule::storeMainFields() {

  nvs.putBool("enable", tmpCfg["enable"].as<bool>());
  nvs.putInt("uiOrder", tmpCfg["uiOrder"].as<int>());
  nvs.putInt("schema", SWITCH_SCHEMA_VERSION);
  nvs.putString("identifier", tmpCfg["identifier"].as<String>());

  LOGI("Applying main data don't require a reboot");
  uiOrder = tmpCfg["uiOrder"].as<int>();
  identifier = tmpCfg["identifier"].as<String>();
}

void SwitchModule::storeSwitches() {
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
    nvs.putString(key, swString);
  }

  nvs.putInt("cfg_sw", id + 1);
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


/* unsafe private method
id is not checked by this method
*/
bool SwitchModule::is_Writable(int id) {

  int validID = isValidID(id);

  if(validID != 1){ return validID; }


  if (Switches[id]->getType() == Type::Input)
  {
    return false;
  }
  
  return true;
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