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
  calibratorConfig calibrator;
  coverConfig cover;
  coverCSaveConfigStruct save;
  coverCLoadConfigStruct load;
};

/* COMMANDS */
struct calibratorCommand{
  bool change;
  unsigned int brightness; 
};


struct coverCommand{
  bool move;
  int angle;
};

struct coverCalibratorsCommand{
  calibratorCommand calibrator;
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
  unsigned int actualBrightness;
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

/* ALPACA*/

struct alpacaCalibratorParameters{
  bool exist;
  unsigned int brightness;
};

struct coverCalibrator{
  coverCalibratorStatus status;
  coverCalibratorsCommand command;
  structCoverCalibratorConfig config;
  alpacaCalibratorParameters alpaca;
  
};

coverCalibrator CoverC;
JsonDocument CoverCConfigTmp;

#endif