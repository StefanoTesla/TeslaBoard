#ifndef SWITCH_VARIABLE
#define SWITCH_VARIABLE


#define _MAX_SWITCH_ID_ 15

/* CONFIGURATION AREA */

struct switchSaveConfigStruct{
  bool execute = false;
  bool failed = false;
  bool restartNeeded = false;
};

struct switchLoadConfigStruct{
  bool isValid = false;
};


/* SINGLE SWITCH */

enum SwitchType{
  SwTypeNull,
  SwTypeDInput,
  SwTypeDOutput,
  SwTypePWM,
  SwTypeServo,
};



struct SwitchConfiStruct{
  unsigned int configuredSwitch;
  switchSaveConfigStruct save;
  switchLoadConfigStruct load;
};


/* SWITCH STRUCT */
struct SwitchStruct{
  SwitchConfiStruct config;
} Switch;


IOBase* SwitchObjects[_MAX_SWITCH_ID_] = {nullptr};
JsonDocument tmpSwitchCfg;


#endif