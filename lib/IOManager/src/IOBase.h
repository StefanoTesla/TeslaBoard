#ifndef IOBASE_H
#define IOBASE_H

#include <Arduino.h>
#include "IOConfigStruct.h"
#include <ArduinoJson.h>

class IOBase {
protected:
    int pin;
    char Name[21];
    char Description[21];
    int type;
    unsigned int min;
    unsigned int max;

public:
    IOBase() {}
    virtual void setup(IOConfigBase* config){};
    virtual void jsonSetup(JsonObject obj){};
    void jsonSetup(JsonObject setup,bool HS = false);
    void getConfiguration(JsonObject cfg);
    int validateJsonCfg(JsonObject obj);
    void copyJsonCfg(JsonObject obj,JsonObject dest);
    virtual int write(int _value) = 0;
    virtual int status();
    virtual int getType(){
        return 0;
    }
    virtual unsigned int getMin(){
        return min;
    }
    virtual unsigned int getMax(){
        return max;
    }
    virtual int getPinNumber(){
        return pin;
    }
    virtual void setName(const char* _name){
       strncpy(Name, _name, sizeof(Name) - 1);
       Name[sizeof(Name) - 1] = '\0';
    }
    virtual void setDescription(const char* _description){
       strncpy(Description, _description, sizeof(Description) - 1);
       Description[sizeof(Description) - 1] = '\0';
    }

    virtual const char* getName(){
        return Name;
    }
    virtual const char* getDescription(){
        return Description;
    }
    virtual ~IOBase() {}

private:
        virtual int readPin() = 0;
};

#endif