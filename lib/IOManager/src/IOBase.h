#ifndef IOBASE_H
#define IOBASE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>

class IOBase {
protected:
    int pin = -1;

    char Name[31] = "";
    char Description[31] = "";
    char uniqueId[9] = "";

    int type = 0;
    unsigned int min = 0;
    unsigned int max = 1;

    // Carica le proprietà comuni dal JSON.
    void setupCommonJson(JsonObjectConst obj);

    // Scrive le proprietà comuni nel JSON di configurazione.
    void getCommonConfiguration(JsonObject cfg);

    // Copia le proprietà comuni durante la sanificazione.
    static void copyCommonJsonCfg(
        JsonObjectConst src,
        JsonObject dest
    );

public:
    IOBase() {}

    virtual bool jsonSetup(
        JsonObjectConst obj,
        bool HS = false
    ) {
        return false;
    }

    virtual void getConfiguration(JsonObject cfg);

    static int validateJsonCfg(JsonObject obj);
    static void copyJsonCfg(JsonObject obj, JsonObject dest);

    virtual int write(int value) = 0;
    virtual int status();

    virtual int getType() {
        return type;
    }

    unsigned int getMin() const {
        return min;
    }

    unsigned int getMax() const {
        return max;
    }

    int getPinNumber() const {
        return pin;
    }

    void setName(const char* name);
    void setDescription(const char* description);

    const char* getName() const {
        return Name;
    }

    const char* getDescription() const {
        return Description;
    }

    const char* getUniqueId() const {
        return uniqueId;
    }

    void setUniqueId(const char* id);

    virtual void loop() {}

    virtual void setDelays(
        unsigned int dOn,
        unsigned int dOff
    ) {}

    virtual void setInvert(bool invert) {}
    virtual void setMoveTime(unsigned int newTime) {}

    virtual ~IOBase() {}

private:
    virtual int readPin() = 0;
};

#endif