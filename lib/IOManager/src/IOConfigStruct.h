#ifndef IOCONFIG_H
#define IOCONFIG_H

struct IOConfigBase {
    virtual ~IOConfigBase() {}
    virtual int getType() const = 0;  // Aggiungi un metodo per ottenere il tipo
};

struct DigitalInputConfig : public IOConfigBase {
    int pin;
    unsigned  invert =0; // 0=N.O. 1=N.C.
    unsigned long dOn =0;
    unsigned long dOff =0;
    int getType() const override { return 1; }
};

struct DigitalOutputConfig : public IOConfigBase {
    int pin;
    unsigned  invert =0; // 0=H_ACT 1=L_ACT
    int getType() const override { return 2; }
};

struct PWMOutputConfig : public IOConfigBase {
    int pin;
    unsigned int channel;

    int getType() const override { return 3; } 
};

struct ServoOutputConfig : public IOConfigBase {
    unsigned int pin;
    unsigned int channel;
    unsigned int maxDeg;
    unsigned int closeDeg;
    unsigned int openDeg;
    unsigned long movTime;
    int getType() const override { return 4; } 
};


#endif