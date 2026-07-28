#include "BaseModule.h"

#undef LOG_TAG
#define LOG_TAG "BaseModule"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

bool BaseModule::openForRead() {
    return NvsManager::getInstance().openNVS(true, schemaName());
}

bool BaseModule::openForWrite() {
    return NvsManager::getInstance().openNVS(false, schemaName());
}

void BaseModule::closeNVS() {
    NvsManager::getInstance().closeNVS();
}

bool BaseModule::initNVS() {
    LOGW("Initializing namespace with default values");

    if (!openForWrite()) {
        LOGE("Unable to open namespace in write mode");
        closeNVS();
        return false;
    }

    NvsManager::getInstance().putBool("enable", defaultEnable());
    NvsManager::getInstance().putInt("order", defaultUiOrder());
    NvsManager::getInstance().putInt("schema", 0);
    NvsManager::getInstance().putString("identifier", defaultIdentifier());

    initSecondaryData();

    closeNVS();
    return true;
}

void BaseModule::begin() {
    LOGI("Loading configuration");

    if (!openForRead()) {
        LOGW("Namespace not found, trying initialization");
        if (!initNVS()) {
            LOGE("NVS initialization failed");
            return;
        }
    }

    if (!openForRead()) {
        LOGE("Critical, unable to open namespace after initialization");
        return;
    }

    uint16_t storedSchema = NvsManager::getInstance().getInt("schema", 0);
    LOGD("Stored schema version: %u", storedSchema);

    closeNVS();

    while (storedSchema < schemaVersion()) {
        LOGW("Schema upgrade required: %u -> %u", storedSchema, schemaVersion());

        if (!applySchemaUpgradeStep(storedSchema)) {
            LOGE("Schema upgrade failed at version %u", storedSchema);
            return;
        }

        if (!openForRead()) {
            LOGE("Unable to reopen namespace after schema upgrade");
            return;
        }

        uint16_t newSchema = NvsManager::getInstance().getInt("schema", storedSchema);
        closeNVS();

        if (newSchema <= storedSchema) {
            LOGE("Schema upgrade did not advance version, aborting");
            return;
        }

        storedSchema = newSchema;
    }

    if (!openForRead()) {
        LOGE("Critical, unable to open namespace for main data load");
        return;
    }

    moduleEnable = NvsManager::getInstance().getBool("enable", defaultEnable());
    uiOrder = NvsManager::getInstance().getInt("order", defaultUiOrder());
    identifier = NvsManager::getInstance().getString("identifier", defaultIdentifier());

    onAfterMainLoad();

    if (!moduleEnable) {
        LOGW("Module not enabled, setup completed");
        closeNVS();
        return;
    }

    loadSecondaryData();

    closeNVS();

    onAfterFullLoad();
}

void BaseModule::getConfiguration(JsonObject dest) {
    dest["enable"] = moduleEnable;
    dest["uiOrder"] = uiOrder;
    dest["identifier"] = identifier;
    dest["reboot"] = rebootNeeded;

    appendSecondaryConfig(dest);
}

bool BaseModule::validateMainFields(const JsonObject &toBeValidated, JsonObject response) {
    JsonArray err = response["errors"].to<JsonArray>();

    if (!toBeValidated["enable"].is<bool>()) {
        LOGE("Enable is not a boolean");
        err.add("Enable is not a boolean");
        return false;
    }

    if (moduleEnable != toBeValidated["enable"].as<bool>()) {
        LOGW("Enable changed, reboot requested");
        response["reboot"] = true;
    }

    if (!toBeValidated["uiOrder"].is<unsigned int>() && !toBeValidated["uiOrder"].is<int>()) {
        LOGE("uiOrder is not a number");
        err.add("uiOrder is not a number");
        return false;
    }

    if (!toBeValidated["identifier"].is<const char*>()) {
        LOGE("Identifier is not a string");
        err.add("Identifier is not a string");
        return false;
    }

    tmpCfg["enable"] = toBeValidated["enable"].as<bool>();
    tmpCfg["uiOrder"] = toBeValidated["uiOrder"].as<unsigned int>();
    tmpCfg["identifier"] = toBeValidated["identifier"].as<const char*>();

    return true;
}

void BaseModule::validateConfiguration(const JsonObject &toBeValidated, JsonObject response) {
    LOGI("Validating configuration");

    tmpCfg.clear();
    response["reboot"] = false;
    response["errors"].to<JsonArray>();

    if (!validateMainFields(toBeValidated, response)) {
        return;
    }

    if (!toBeValidated["enable"].as<bool>()) {
        LOGI("Module disabled in incoming config, stopping validation");
        rebootNeeded = response["reboot"].as<bool>();
        validConfig = true;
        return;
    }

    validConfig = validateSecondaryConfig(toBeValidated, response);
    rebootNeeded = response["reboot"].as<bool>();
}

void BaseModule::storeMainFields() {
    NvsManager::getInstance().putBool("enable", tmpCfg["enable"].as<bool>());
    NvsManager::getInstance().putInt("order", tmpCfg["uiOrder"].as<unsigned int>());
    NvsManager::getInstance().putInt("schema", schemaVersion());
    NvsManager::getInstance().putString("identifier", tmpCfg["identifier"].as<const char*>());

    moduleEnable = tmpCfg["enable"].as<bool>();
    uiOrder = tmpCfg["uiOrder"].as<unsigned int>();
    identifier = tmpCfg["identifier"].as<const char*>();
}

void BaseModule::storeConfiguration(const JsonObject &toBeStored) {
    LOGI("Writing new configuration on NVS");

    tmpCfg.clear();
    tmpCfg["enable"] = toBeStored["enable"].as<bool>();
    tmpCfg["uiOrder"] = toBeStored["uiOrder"].as<unsigned int>();
    tmpCfg["identifier"] = toBeStored["identifier"].as<const char*>();

    onBeforeStore();

    if (!openForWrite()) {
        LOGE("Failed to open NVS for writing");
        return;
    }

    storeMainFields();

    if (!tmpCfg["enable"].as<bool>()) {
        LOGI("Module disabled, main configuration stored");
        closeNVS();
        return;
    }

    storeSecondaryConfig(toBeStored);

    closeNVS();
}