#ifndef IOCONFIG_H
#define IOCONFIG_H

struct IOConfigBase {
    virtual ~IOConfigBase() {}
    virtual int getType() const = 0;  // Aggiungi un metodo per ottenere il tipo
};

struct DigitalInputConfig : public IOConfigBase {
    int pin;
    bool invert;
    unsigned long dOn;
    unsigned long dOff;

    int getType() const override { return 1; } // Restituisci un tipo univoco per DigitalInputConfig
};
struct AnalogInputConfig : public IOConfigBase {
    int pin;
    bool invert;
    unsigned long ton;
    unsigned long toff;

    int getType() const override { return 2; } // Restituisci un tipo univoco per DigitalInputConfig
};
struct DigitalOutputConfig : public IOConfigBase {
    int pin;
    bool invert;
    unsigned long ton;
    unsigned long toff;

    int getType() const override { return 3; } // Restituisci un tipo univoco per DigitalInputConfig
};
struct PWMOutputConfig : public IOConfigBase {
    int pin;
    int channel;

    int getType() const override { return 4; } // Restituisci un tipo univoco per DigitalInputConfig
};
struct ServoutputConfig : public IOConfigBase {
    int pin;
    int channel;
    unsigned int maxDeg;
    unsigned long closeDeg;
    unsigned long openDeg;

    int getType() const override { return 5; } // Restituisci un tipo univoco per DigitalInputConfig
};


#endif