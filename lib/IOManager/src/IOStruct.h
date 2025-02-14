#ifndef IOCONFIG_H
#define IOCONFIG_H

struct IOConfigBase {
    virtual ~IOConfigBase() {}
    virtual int getType() const = 0;  // Aggiungi un metodo per ottenere il tipo
};

struct DigitalInputConfig : public IOConfigBase {
    int pin;
    bool invert;
    unsigned long ton;
    unsigned long toff;

    int getType() const override { return 1; } // Restituisci un tipo univoco per DigitalInputConfig
};
struct DigitalOutputConfig : public IOConfigBase {
    int pin;

    int getType() const override { return 3; } // Restituisci un tipo univoco per DigitalInputConfig
};
struct PWMConfig : public IOConfigBase {
    int pin;
    int channel;

    int getType() const override { return 4; } // Restituisci un tipo univoco per DigitalInputConfig
};
struct ServoConfig : public IOConfigBase {
    int pin;
    bool invert;
    unsigned int maxDeg;
    unsigned int closeDeg;
    unsigned int openDeg;

    int getType() const override { return 5; } // Restituisci un tipo univoco per DigitalInputConfig
};


#endif