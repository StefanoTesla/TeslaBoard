#ifndef BASEMODULE_H
#define BASEMODULE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <NVSManager.h>
#include <esp_log.h>

class BaseModule {
public:
    virtual ~BaseModule() = default;

    virtual void begin();
    bool isEnable() const { return moduleEnable; }

    virtual void getConfiguration(JsonObject dest);
    virtual void validateConfiguration(const JsonObject &toBeValidated, JsonObject response);
    virtual void storeConfiguration(const JsonObject &toBeStored);

    String getIdentifier() const { return identifier; }
    unsigned int getUiOrder() const { return uiOrder; }
    bool isRebootNeeded() const { return rebootNeeded; }

protected:
    bool moduleEnable = false;
    bool validConfig = false;
    bool rebootNeeded = false;
    unsigned int uiOrder = 1;
    String identifier = "";

    JsonDocument tmpCfg;

protected:
    virtual const char* schemaName() const = 0;
    virtual uint16_t schemaVersion() const = 0;
    virtual const char* defaultIdentifier() const = 0;
    virtual bool defaultEnable() const { return false; }
    virtual unsigned int defaultUiOrder() const { return 1; }

    virtual void initSecondaryData() = 0;
    virtual void loadSecondaryData() = 0;
    virtual void appendSecondaryConfig(JsonObject dest) = 0;
    virtual bool validateSecondaryConfig(const JsonObject &toBeValidated, JsonObject response) = 0;
    virtual void storeSecondaryConfig(const JsonObject &toBeStored) = 0;

    virtual bool applySchemaUpgradeStep(uint16_t currentVersion) = 0;

    virtual void onAfterMainLoad() {}
    virtual void onAfterFullLoad() {}
    virtual void onBeforeStore() {}

protected:
    bool initNVS();
    bool validateMainFields(const JsonObject &toBeValidated, JsonObject response);
    void storeMainFields();

    bool openForRead();
    bool openForWrite();
    void closeNVS();
};

#endif