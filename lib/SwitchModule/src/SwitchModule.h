#ifndef SWITCH_H
#define SWITCH_H

#include "esp_log.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <DigitalInput/DigitalInput.h>
#include <DigitalOutput/DigitalOutput.h>
#include <IOManager.h>
#include <PWM/PWMOutput.h>
#include <Servo/ServoOutput.h>
#include <VirtualInput/VirtualInput.h>
#include <PWMManager.h>
#include <BaseModule.h>
#include <NVSManager.h>

#define SWITCH_SCHEMA_VERSION 1
#define SWITCH_SCHEMA_NAME "switchcfg"
#define SWITCH_MAX_SWITCHES 21

class SwitchModule : public BaseModule {
public:
  enum Type { NotPresent, Input, Output, PWM, Servo, Virtual };

public:
  SwitchModule(PWMManager *channelManager) : chMgr(channelManager) {}

  void loop();

  int getConfiguredSwitch(){return configuredSwitches;}
  bool handlePacket(char* payload, Stream& out);
  void reportSwitchState(int id,JsonObject state);
  
  int isValidID(int id);
  int isWritable(int id);
  int isValidValue(int id, int value);
  int setSwitchValue(int id, int state);
  int setVirtualSwitchValue(int id, int state);
  int setServoPositionAsync(int id,int position);
  int getServoIsMoving(int id);
  int getSwitchState(int id);
  int getType(int id);
  int getMax(int id);
  int getMin(int id);
  int findSwitchByUid(const char* uid) const;
  const char* getSwitchName(int id);
  const char* getSwitchDescription(int id);


protected:
  const char* schemaName() const override { return SWITCH_SCHEMA_NAME; }
  uint16_t schemaVersion() const override { return SWITCH_SCHEMA_VERSION; }
  const char* defaultIdentifier() const override { return "Switch"; }
  void initSecondaryData() override;
  void loadSecondaryData() override;
  void appendSecondaryConfig(JsonObject dest) override;
  bool validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) override;
  void storeSecondaryConfig(const JsonObject &toBeStored) override;
  bool applySchemaUpgradeStep(uint16_t currentVersion) override;

private:
  JsonDocument tmpCfg;
  PWMManager *chMgr;
  IOBase *Switches[SWITCH_MAX_SWITCHES] = {nullptr};
  static char _deviceStateBuffer[512];
  unsigned int configuredSwitches;

  void validateSwitches(const JsonArray &switches, JsonObject response);
  int validateSwitchType(Type type, const JsonObject &singleSW);
  void checkIfRebootNeeded(int id, Type type, const JsonObject &singleSW,
                           JsonObject response);
  static constexpr size_t UID_LENGTH = 8;

  static void generateSwitchUid(char uid[UID_LENGTH + 1]);

  static bool validSwitchUid(const char* uid);

  static bool uidAlreadyUsed(
        JsonArray switches,
        const char* uid
    );

  bool uidAlreadyUsedInOldConfiguration(
      const char* uid,
      int exceptIndex
  ) const;

  enum class SwitchSerialCommand : uint8_t {
    Unknown = 0,
    MaxSwitch,
    Desc,
    IntVersion,
    Name,
    SupportedActions,
    Action,
    CanAsync,
    CancelAsync,
    CanWrite,
    CmdBlind,
    CmdBool,
    CmdString,
    Connect,
    Disconnect,
    Connected,
    Connecting,
    GetSwitchValue,
    GetSwitchDesc,
    GetSwitchName,
    GetSwitchMax,
    GetSwitchMin,
    SetAsync,
    SetAsyncVal,
    StateChangeComplete,
    Set,
    SetName,
    SetDesc,
    SetValue,
    Step,
    DeviceState
  };

  SwitchSerialCommand parseCommand(const char* cmd);        
};

#endif