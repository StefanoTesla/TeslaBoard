#ifndef SWITCH_H
#define SWITCH_H

#include "esp_log.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <DigitalInput/DigitalInput.h>
#include <DigitalOutput/DigitalOutput.h>
#include <IOManager.h>
#include <PWM/PWMOutput.h>
#include <PWMManager.h>
#include <Preferences.h>

#define SWITCH_SCHEMA_VERSION 1
#define SWITCH_SCHEMA_NAME "switchcfg"
#define SWITCH_MAX_SWITCHES 21

class SwitchModule {
public:
  enum Type { NotPresent, Input, Output, PWM, Servo };

public:
  SwitchModule(PWMManager *channelManager) : chMgr(channelManager) {}

  void begin();
  bool isEnable() { return moduleEnable; };
  void loop();

  void getConfiguration(JsonObject dest);
  void validateConfiguration(const JsonObject &incomingData, JsonObject response);
  void storeConfiguration();
  String getIdentifier() { return identifier; }


  int getConfiguredSwitch(){return configuredSwitches;}

  void reportSwitchState(int id,JsonObject state);
  
  int isValidID(int id);
  int isWritable(int id);
  int isValidValue(int id, int value);
  int setSwitchValue(int id, int state);
  int setServoPositionAsync(int id,int position);
  int getServoIsMoving(int id);
  int getSwitchState(int id);
  int getType(int id);
  int getMax(int id);
  int getMin(int id);

  const char* getSwitchName(int id);
  const char* getSwitchDescription(int id);

  unsigned int uiOrder;

private:
  /* functions to handle the configuration */

  JsonObject tmpLoad;
  JsonDocument tmpCfg;
  bool moduleEnable = false;
  bool validConfig = false;
  bool rebootNeeded = false;
private:
  PWMManager *chMgr;
  String identifier = "Switch";
  IOBase *Switches[SWITCH_MAX_SWITCHES] = {nullptr};
  Preferences nvs;

  enum PrefEnumStatus { CLOSED, OPEN_WRITE, OPEN_READOLNY };
  PrefEnumStatus nvsStatus = CLOSED;
  unsigned int configuredSwitches;

  bool openNVS(bool readOnly);
  void closeNVS();
  bool initNVS();
  void updateNVS1();
  
  bool validateMainFields(const JsonObject &data, JsonObject response);
  void validateSwitches(const JsonArray &switches, JsonObject response);
  int validateSwitchType(Type type, const JsonObject &singleSW);
  void checkIfRebootNeeded(int id, Type type, const JsonObject &singleSW,
                           JsonObject response);

  void storeMainFields();
  void storeSwitches();

 

};

#endif