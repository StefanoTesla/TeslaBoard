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

struct SwitchCommandStruct {
  bool execute;
  bool boValue;
  int intValue;
};

struct SwitchProperty{
  SwitchType type;
  char Name[21];
  char Description[21];
  int minValue = 0;
  int maxValue = 1;
  int pwmch;
  unsigned int pin;
};

struct SwitchActualValue{
  bool boValue;  //must be true also if is an int value value > minValue, otherwise false
  int intValue; //must return the integer value of the switch, 1 is return if bool is true
};

struct SwitchArrayStruct
{
  SwitchProperty property;
  SwitchCommandStruct command; 
  SwitchActualValue actualValue;
};

struct SwitchConfiStruct{
  unsigned int configuredSwitch;
  //SwitchArrayStruct tmp[_MAX_SWITCH_ID_];
  switchSaveConfigStruct save;
  switchLoadConfigStruct load;
};


/* SWITCH STRUCT */
struct SwitchStruct{
  SwitchConfiStruct config;
  SwitchArrayStruct data[_MAX_SWITCH_ID_];
  
} Switch;


IOBase* SwitchObjects[_MAX_SWITCH_ID_] = {nullptr};
JsonDocument tmpSwitchCfg;


#endif