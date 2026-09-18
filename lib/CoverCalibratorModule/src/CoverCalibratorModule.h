#ifndef COVERCMODULE_H
#define COVERCMODULE_H

#include <Arduino.h>
#include "Cover/Cover.h"
#include "Calibrator/Calibrator.h"
#include <ArduinoJson.h>
#include <PWMManager.h>
#include <BaseModule.h>
#include <NVSManager.h>

#define COVERC_SCHEMA_VERSION 1
#define COVERC_SCHEMA_NAME "cccfg"

class CoverCalibratorModule : public BaseModule {
public:
    CoverCalibratorModule(PWMManager* channelManager) :
        chMgr(channelManager),       
        cover(channelManager),       
        calibrator(channelManager)      
    {}
    Cover cover;
    Calibrator calibrator;

    void loop();
    bool handlePacket(char* payload, Stream& out);

protected:
    const char* schemaName() const override { return COVERC_SCHEMA_NAME; }
    uint16_t schemaVersion() const override { return COVERC_SCHEMA_VERSION; }
    const char* defaultIdentifier() const override { return "CoverCal"; }

    void initSecondaryData() override;
    void loadSecondaryData() override;
    void appendSecondaryConfig(JsonObject dest) override;
    bool validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) override;
    void storeSecondaryConfig(const JsonObject &toBeStored) override;
    bool applySchemaUpgradeStep(uint16_t currentVersion) override;

private:
    PWMManager* chMgr;
    
    enum class CCSerialCommand : uint8_t {
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
        Brightness,
        MaxBrightness,
        CalibratorChanging,
        CalibratorState,
        CalibratorOff,
        CalibratorOn, 
        CoverMoving,
        CoverState,
        CloseCover,
        OpenCover,
        HaltCover
  };

  CCSerialCommand parseCommand(const char* cmd);  


  bool isCalibratorCommand(CCSerialCommand cmd);
  bool isCoverCommand(CCSerialCommand cmd);

};

#endif