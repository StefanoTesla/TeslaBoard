#include "IOBase.h"

void IOBase::setName(const char* name) {
    if (name == nullptr) {
        Name[0] = '\0';
        return;
    }

    strncpy(Name, name, sizeof(Name) - 1);
    Name[sizeof(Name) - 1] = '\0';
}

void IOBase::setDescription(const char* description) {
    if (description == nullptr) {
        Description[0] = '\0';
        return;
    }

    strncpy(Description, description, sizeof(Description) - 1);
    Description[sizeof(Description) - 1] = '\0';
}

void IOBase::setUniqueId(const char* id) {
    if (id == nullptr) {
        uniqueId[0] = '\0';
        return;
    }

    strncpy(uniqueId, id, sizeof(uniqueId) - 1);
    uniqueId[sizeof(uniqueId) - 1] = '\0';
}

void IOBase::setupCommonJson(JsonObjectConst obj) {
    setName(obj["name"] | "");
    setDescription(obj["desc"] | "");

    const char* id = obj["uniqueId"] | "";
    setUniqueId(id);
}

void IOBase::getCommonConfiguration(JsonObject cfg) {
    cfg["name"] = Name;
    cfg["desc"] = Description;
    cfg["uniqueId"] = uniqueId;
}

void IOBase::copyCommonJsonCfg( JsonObjectConst src, JsonObject dest) {
    dest["name"] = src["name"] | "";
    dest["desc"] = src["desc"] | "";
    dest["uniqueId"] = src["uniqueId"] | "";
}