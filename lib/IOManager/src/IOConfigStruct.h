#ifndef IOCONFIG_H
#define IOCONFIG_H

struct IOConfigBase {
    virtual ~IOConfigBase() {}
    virtual int getType() const = 0;  // Aggiungi un metodo per ottenere il tipo
};

struct DigitalInputConfig : public IOConfigBase {
    int pin;
    int invert; // 0=N.O. 1=N.C.
    int dOn;
    int dOff;
    int getType() const override { return 1; }
};

struct DigitalOutputConfig : public IOConfigBase {
    int pin;
    int invert; // 0=H_ACT 1=L_ACT
    int getType() const override { return 2; }
};

struct PWMOutputConfig : public IOConfigBase {
    int pin;
    int channel;

    int getType() const override { return 3; } 
};

struct ServoOutputConfig : public IOConfigBase {
    int pin;
    int channel;
    int maxDeg;
    int closeDeg;
    int openDeg;
    int movTime;
    int getType() const override { return 4; } 
};


#endif