#ifndef DOME_H
#define DOME_H

#include <Arduino.h>
#include "Shutter/Shutter.h"
#include <ArduinoJson.h>
#include "esp_log.h"
#include <NVSManager.h>
#include <BaseModule.h>

#define DOME_SCHEMA_VERSION 1
#define DOME_SCHEMA_NAME "domecfg"

class DomeModule : public BaseModule {
public:
    Shutter shutter;

    DomeModule() = default;
    bool isEnable();
    void loop();
    bool handlePacket(char* payload, Stream& out);
    
protected:
    const char* schemaName() const override { return DOME_SCHEMA_NAME; }
    uint16_t schemaVersion() const override { return DOME_SCHEMA_VERSION; }
    const char* defaultIdentifier() const override { return "Dome"; }
    void initSecondaryData() override;
    void loadSecondaryData() override;
    void appendSecondaryConfig(JsonObject dest) override;
    bool validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) override;
    void storeSecondaryConfig(const JsonObject &toBeStored) override;
    bool applySchemaUpgradeStep(uint16_t currentVersion) override;

private:

    enum class DomeSerialCommand : uint8_t {
        Unknown = 0,
        Name,//
        Desc,//
        IntVersion,//
        SupportedActions,//
        Action,//
        CmdBlind,//
        CmdBool,//
        CmdString,//
        Connect,//
        Disconnect,//
        Connected,//
        Connecting,//
        AbortSlew,//
        DeviceState,
        CanSetShutter,//
        OpenShutter,//
        CloseShutter,//
        ShutterStatus,//
        Slaved,//
        Slewing,//
        Altitude,//
        Azimuth,//
        AtHome,//
        AtPark,//
        SlewToAltitude,//
        SlewToAzimuth,//
        CanFindHome,//
        CanSetAzi,//
        CanSetPark,//
        CanSlave,//
        CanSyncAzimuth,//
        FindHome,//
        Park,//
        SetPark,//

    };


    DomeSerialCommand parseCommand(const char* cmd);  
};

#endif