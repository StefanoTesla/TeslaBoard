#ifndef IOCONFIG_H
#define IOCONFIG_H

struct IOConfigBase {
    virtual ~IOConfigBase() {}
    virtual int getType() const = 0;  // Aggiungi un metodo per ottenere il tipo
};

struct DigitalInputConfig : public IOConfigBase {
    int pin;
    bool invert =0; // 0=N.O. 1=N.C.
    unsigned long dOn =0;
    unsigned long dOff =0;
    int getType() const override { return 1; }
};

struct DigitalOutputConfig : public IOConfigBase {
    int pin;
    bool invert =0; // 0=H_ACT 1=L_ACT
    int getType() const override { return 2; }
};

struct PWMOutputConfig : public IOConfigBase {
    int pin;
    bool fastPWM = false;
    int ledChannel;
    int getType() const override { return 3; } 
};

struct ServoOutputConfig : public IOConfigBase {
    int ledChannel;
    unsigned int pin;
    unsigned int maxDeg;
    unsigned int closeDeg;
    unsigned int openDeg;
    unsigned long moveTime;
    int getType() const override { return 4; } 
};


#endif