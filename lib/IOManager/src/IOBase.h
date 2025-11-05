#ifndef IOBASE_H
#define IOBASE_H

#include <Arduino.h>

#include <ArduinoJson.h>

class IOBase {
protected:
    int pin;
    char Name[21] ="";
    char Description[21]="";
    int type;
    unsigned int min = 0;
    unsigned int max = 1;

public:
    IOBase() {}
    virtual bool jsonSetup(JsonObjectConst obj, bool HS=false){ return false;};
    virtual void getConfiguration(JsonObject cfg);
    static int validateJsonCfg(JsonObject obj);
    static void copyJsonCfg(JsonObject obj,JsonObject dest);
    virtual int write(int _percentage) = 0;
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
    virtual void loop(){}

    virtual void setDelays(unsigned int dOn,unsigned int dOff){};
    virtual void setInvert(bool invert){};
    virtual void setMoveTime(unsigned int newTime){};
    virtual ~IOBase() {}

private:
        virtual int readPin() = 0;
};

#endif