#ifndef IOBASE_H
#define IOBASE_H

#include <Arduino.h>

class IOBase {
protected:
    int pin;
    char Name[21];
    char Description[21];
    int type;

public:
    IOBase() {}
    virtual void setPin(int _pin){}
    virtual void setChannel(int _pin){}
    virtual void setup(){}
    virtual int write(int _value) = 0;
    virtual int readPin() = 0;
    virtual int status();
    int getPinNumber(){
         return pin;
    }
    void setName(const char* _name){
        strncpy(Name, _name, sizeof(Name) - 1);
        Name[sizeof(Name) - 1] = '\0';
    }
    void setDescriprion(const char* _description){
        strncpy(Description, _description, sizeof(Description) - 1);
        Description[sizeof(Description) - 1] = '\0';
    }
    virtual int getType();
    virtual ~IOBase() {}
};

#endif