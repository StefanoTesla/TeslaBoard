#include "SwitchModule.h"
#undef LOG_TAG
#define LOG_TAG "Switch"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

#pragma region Configuration
/* initialize the switches */
void SwitchModule::begin() {
  LOGI("Loading configuration");
  JsonDocument doc;
  Preferences pref;

  bool nvsStop = false;

  if (!pref.begin(SWITCH_SCHEMA_NAME)) {
    LOGE("Error loading switch nvs partition, trying to format it");
    pref.end();
    nvsStop = true;
    initNVS();
    if (!pref.begin(SWITCH_SCHEMA_NAME)) {
      LOGE("Critical, unable to loading switches nvs after initialization");
      pref.end();
      return;
    } else {
      pref.end();
      nvsStop = true;
    }
  }
  LOGV("Namespace open without problem");

  if (nvsStop) {
    pref.begin(SWITCH_SCHEMA_NAME);
  }

  int schemaVersion = pref.getInt("schema", 0);
  LOGD("schema version is: %d", schemaVersion);

  if (schemaVersion < SWITCH_SCHEMA_VERSION) {
    LOGV("pref.end");
    pref.end(); // since the upgrade operation required to open the nvs with
                // write rigths I close it for reopen again in read only leater
    nvsStop = true;
    LOGW("schema need an upgrade");
    switch (schemaVersion) {
    case 0:
      LOGI("upgrading from 0 to 1");
      updateNVS1();
      break;

    default:
      break;
    }
  }

  if (nvsStop) {
    LOGV("nvs was stopped previusly, reopening it");
    if (!pref.begin(SWITCH_SCHEMA_NAME)) {
      moduleEnable = false;
      LOGE("Error loading switch nvs partition, stop");
      return;
    };
  }

  moduleEnable = pref.getBool("enable", false);
  uiOrder = pref.getInt("order", 1);
  identifier = pref.getString("identifier", "Switch");

  if (!moduleEnable) {
    LOGW("Module not enable, setup completed");
    pref.end();
    return;
  }

  configuredSwitches = pref.getInt("cfg_sw", 0);

  if (configuredSwitches == 0) {
    LOGI("Any switches configured yet, disabling the module...");
    moduleEnable = false;
    return;
  }

  for (size_t i = 0; i < configuredSwitches; i++) {
    tmpCfg.clear();
    // prepare the page key "swX"
    char key[10];
    sprintf(key, "sw%d", i);
    // get the string stored
    String swString;
    swString = pref.getString(key, "{\"type\":0}");
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
  pref.end();
  LOGV("Switch begin finished");
}

void SwitchModule::updateNVS1() {
  Preferences pref;
  LOGI("upgrading NVS from 0 to 1");
  if (pref.begin(SWITCH_SCHEMA_NAME)) {
    pref.putBool("enable", false);
    pref.putInt("schema", SWITCH_SCHEMA_VERSION);
    pref.putString("identifier", "Switch");
    pref.putInt("order", 1);

    pref.end();
  } else {
    LOGE("Unable to open the name space for the upgrade");
  }
}

void SwitchModule::initNVS() {
  Preferences pref;
  LOGI("initNVS() initialization begin");
  if (!pref.begin(SWITCH_SCHEMA_NAME, false)) {
    LOGE("initNVS() failed to open the namespace with write rights, "
         "initialization failed");
    pref.end();
    return;
  };

  LOGV("Storing default keys");
  pref.putBool("enable", false);
  pref.putInt("order", 1);
  pref.putInt("schema", 1);
  pref.putString("identifier", "Switch");
  pref.putInt("cfg_sw", 0);
  for (int i = 0; i < SWITCH_MAX_SWITCHES; i++) {
    char key[10];
    sprintf(key, "sw%d", i);
    pref.putString(key, "{\"type\":0}");
  }

  pref.end();

  LOGI("initNVS() end");
}

void SwitchModule::getConfiguration(JsonObject dest) {

  dest["enable"] = moduleEnable;
  dest["uiOrder"] = uiOrder;
  dest["identifier"] = identifier;

  JsonArray switchesArray = dest["Switches"].to<JsonArray>();

  for (size_t i = 0; i < SWITCH_MAX_SWITCHES; i++) {
    if (Switches[i] == nullptr || Switches[i]->getType() > Type::Servo) {
      continue;
    }

    JsonObject swi;
    Switches[i]->getConfiguration(swi);
    switchesArray.add(swi);
  }
}

void SwitchModule::validateConfiguration(const JsonObject &toBeValidated,
                                         JsonObject response) {
  LOGI("data validation");
  response["reboot"] = false;

  JsonArray err = response["errors"].to<JsonArray>();
  if (!toBeValidated["enable"].is<bool>()) {
    LOGE("enable don't exist or is not a boolean");
    err.add("Enable is not a boolean");
    return;
  }

  if (moduleEnable != toBeValidated["enable"]) {
    LOGW("enable is not the same as actual, reboot requested");
    response["reboot"] = true;
  }

  if (!toBeValidated["uiOrder"].is<int>()) {
    LOGE("order for ui don't exist or is not a integer");
    err.add("Order is not a numeber");
    return;
  }

  if (!toBeValidated["identifier"].is<String>()) {
    LOGE("board identifier don't exist or is not a string");
    err.add("Identifier is not a string");
    return;
  }

  if (!toBeValidated["enable"].as<bool>()) {
    LOGI("Main module is not enable, stop validation");
    return;
  }

  LOGI("Main data validation ok, starting with shutter data");

  if (!toBeValidated["Switches"].is<JsonArray>()) {
    err.add("Switches is not an array");
    return;
  }

  int id = -1;
  for (JsonObject singleSW : toBeValidated["Switches"].as<JsonArray>()) {
    id++;
    int retVal = 0;
    LOGV("In the loop");
    if (!singleSW["type"].is<unsigned int>()) {
      continue;
    }
    Type type = static_cast<Type>(singleSW["type"].as<unsigned int>());

    if (NotPresent == type) {
      continue;
    } else if (Input == type) {
      retVal = DigitalInput::validateJsonCfg(singleSW);
      LOGV("retVal: %d", retVal);
      if (retVal != 1) {
        JsonObject e = err.add<JsonObject>();
        e["id"] = id;
        e["error"] = retVal;
        return;
      }
    } else if (Output == type) {
      retVal = DigitalOutput::validateJsonCfg(singleSW);
      if (retVal != 1) {
        JsonObject e = err.add<JsonObject>();
        e["id"] = id;
        e["error"] = retVal;
        return;
      }
    } else if (PWM == type) {
      retVal = PWMOutput::validateJsonCfg(singleSW);
      if (retVal != 1) {
        JsonObject e = err.add<JsonObject>();
        e["id"] = id;
        e["error"] = retVal;
        return;
      }
    } else if (Servo == type) {
      retVal = ServoOutput::validateJsonCfg(singleSW);
      if (retVal != 1) {
        JsonObject e = err.add<JsonObject>();
        e["id"] = id;
        e["error"] = retVal;
        return;
      }
    } else {
      LOGV("undefind type");
      continue;
    }
  }
}

void SwitchModule::storeConfiguration(JsonObject toBeStored) {
  LOGI("Writing new configuration on the NVS");
  Preferences pref;

  serializeJson(toBeStored, Serial);
  pref.begin(SWITCH_SCHEMA_NAME);

  bool inEnable = toBeStored["enable"].as<bool>();

  pref.putBool("enable", inEnable);
  pref.putInt("uiOrder", toBeStored["uiOrder"].as<int>());
  pref.putInt("schema", SWITCH_SCHEMA_VERSION);
  pref.putString("identifier", toBeStored["identifier"].as<String>());

  /* apply only the changes that don't require a reboot */
  LOGI("Applying data dont require a reboot");
  uiOrder = toBeStored["uiOrder"].as<int>();
  identifier = toBeStored["identifier"].as<String>();

  /* if module is not enable don't write anymore*/
  if (!inEnable) {
    LOGI("Main Module is not enable, writing new configuration done.");
    return;
  }
  LOGI("Main config done, start with switches");

  int id = -1;
  JsonDocument tmpDoc;
  for (JsonObject inSwitch : toBeStored["Switches"].as<JsonArray>()) {
    tmpDoc.clear();
    if (!inSwitch["type"].is<int>()) {
      LOGE("type don't exist");
      continue;
    }

    Type type = static_cast<Type>(inSwitch["type"].as<unsigned int>());
    LOGV("Type is: %d", type);
    if (NotPresent == type) {
      continue;
    } else if (Input == type) {
      id++;
      LOGV("is an input");
      serializeJson(inSwitch, Serial);

      JsonObject tmp = tmpDoc.to<JsonObject>();
      DigitalInput::copyJsonCfg(inSwitch, tmp);

      char key[10];
      sprintf(key, "sw%d", id);
      String swString;
      serializeJson(inSwitch,swString);
      pref.putString(key, swString);
    }
  }
}

#pragma endregion

void SwitchModule::loop() {

  for (size_t i = 0; i < configuredSwitches; i++) {
    if (Switches[i] == nullptr) {
      continue;
    }

    if (Switches[i]->getType() == 2 || Switches[i]->getType() == 4) {
      Switches[i]->loop();
    }
  }
}
