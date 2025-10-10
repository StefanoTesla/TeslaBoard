#ifndef CC_VARIABLE
#define CC_VARIABLE

/* CONFIG*/

struct coverCSaveConfigStruct{
  bool execute = false;
  bool restartNeeded = false;
};

struct coverCLoadConfigStruct{
  bool isValid = false;
};

struct calibratorConfig{
  bool present;
};

struct coverConfig{
  bool present;
};


struct structCoverCalibratorConfig{
  bool isEnable = false;
  int schemaVersion;
  unsigned int order = 1;
  calibratorConfig calibrator;
  coverConfig cover;
  coverCSaveConfigStruct save;
  coverCLoadConfigStruct load;
};

/* COMMANDS */

struct coverCommand{
  bool move;
  int angle;
};

struct coverCalibratorsCommand{
  coverCommand cover;
};


/* STATUS*/
/* CALIBRATOR */
enum calirbatorStatusEnum{
  CalibStatusNoPresent,
  CalibStatusOff,
  CalibStatusNotReady,
  CalibStatusReady,
  CalibStatusUnknow,
  CalibStatusError
};

struct calibratorStatus{
  calirbatorStatusEnum status;
};

/* COVER */
enum coverStatusEnum{
  CoverStatusNoPresent,
  CoverStatusClose,
  CoverStatusMoving,
  CoverStatusOpen,
  CoverStatusUnknow,
  CoverStatusError
};

struct coverStatus{
  coverStatusEnum status;
  int angle;
};

struct coverStruct {
  coverStatus status;
  coverCommand command;
};

struct coverCalibratorStatus{
  calibratorStatus calibrator;
  coverStatus cover;
  unsigned int actualBrightness;
};



struct coverCalibrator{
  coverCalibratorStatus status;
  coverCalibratorsCommand command;
  structCoverCalibratorConfig config;
  
};

coverCalibrator CoverC;
JsonDocument CoverCConfigTmp;

#endif