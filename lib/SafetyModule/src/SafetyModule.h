#ifndef SAFETYMODULE_H
#define SAFETYMODULE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <BaseModule.h>
#include <NVSManager.h>

#define SAFETY_SCHEMA_VERSION 1
#define SAFETY_SCHEMA_NAME "sfcfg"
#define SAFETY_MAX_CONDITIONS 20

class SwitchModule;


class SafetyModule : public BaseModule {
public:
    SafetyModule(SwitchModule* switchModule);

    void loop();
    bool isSafe() { return status == SafetyStatusEnum::Safe;}
    bool handlePacket(char* payload, Stream& out);

protected:
    const char* schemaName() const override { return SAFETY_SCHEMA_NAME; }
    uint16_t schemaVersion() const override { return SAFETY_SCHEMA_VERSION; }
    const char* defaultIdentifier() const override { return "Safety"; }

    void initSecondaryData() override;
    void loadSecondaryData() override;
    void appendSecondaryConfig(JsonObject dest) override;
    bool validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) override;
    void storeSecondaryConfig(const JsonObject &toBeStored) override;
    bool applySchemaUpgradeStep(uint16_t currentVersion) override;

private:
    
    SwitchModule* switches = nullptr;

    enum class SafetySerialCommand : uint8_t {
        Unknown = 0,
        Desc,
        IntVersion,
        Name,
        SupportedActions,
        Action,
        CmdBlind,
        CmdBool,
        CmdString,
        Connect,
        Disconnect,
        Connected,
        Connecting,
        DeviceState,
        IsSafe
    };
    SafetySerialCommand parseCommand(const char* cmd);  

    enum class SafetyStatusEnum : uint8_t {
        Invalid = 0,
        Unknown ,
        Safe,
        Unsafe,
    };

    SafetyStatusEnum status = SafetyStatusEnum::Invalid;




};

#endif