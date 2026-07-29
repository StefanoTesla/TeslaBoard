#ifndef BOARDMODULE_H
#define BOARDMODULE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <BaseModule.h>

#define BOARD_SCHEMA_VERSION 2
#define BOARD_SCHEMA_NAME "boardcfg"

class BoardModule : public BaseModule {
public:
    BoardModule() = default;
    void loop();
    String getLocale() const { return locale; }
    unsigned long getUptime() const { return upTime; }
    bool handlePacket(char* payload, Stream& out);
protected:
    const char* schemaName() const override { return BOARD_SCHEMA_NAME; }
    uint16_t schemaVersion() const override { return BOARD_SCHEMA_VERSION; }
    const char* defaultIdentifier() const override { return "TeslaBoard"; }
    bool defaultEnable() const override { return true; }

    void initSecondaryData() override;
    void loadSecondaryData() override;
    void appendSecondaryConfig(JsonObject dest) override;
    bool validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) override;
    void storeSecondaryConfig(const JsonObject &toBeStored) override;
    bool applySchemaUpgradeStep(uint16_t currentVersion) override;

private:
    String locale = "en";
    unsigned long ackMillis = 0;
    unsigned long upTime = 0;
};

#endif