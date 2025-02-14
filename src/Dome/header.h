#ifndef DOME_VARIABLE
#define DOME_VARIABLE

enum ShInputState {
  ShInputNoOne,
  ShInputOnlyClose,
  ShInputOnlyOpen,
  ShInputAll,
};

enum ShStatus {
  ShStatusOpen,
  ShStatusClose,
  ShStatusOpening,
  ShStatusClosing,
  ShStatusError
};

enum ShCommands {
  ShCommandIdle,
  ShCommandOpen,
  ShCommandClose,
  ShCommandHalt
};


struct ShutterStruct {
  ShInputState input;
  ShStatus status;
  ShCommands command;
  int Cycle;
  bool MoveRetry;
  unsigned int LastDomeCommand;
  unsigned long lastCommunicationMillis;
  unsigned long timeOutAck;
  bool inOpen;
  bool inClose;
};

struct domeHeader{
  InputStructure inOpen;
  InputStructure inClose;
  unsigned int outStart_Open;
  unsigned int outHalt_Close;
  unsigned int movingTimeOut = 20000;
  bool enAutoClose;
  unsigned int autoCloseTimeOut = 20;
};

struct domeSaveConfigStruct{
  bool execute = false;
  bool failed = false;
  bool restartNeeded = false;
};

struct domeLoadConfigStruct{
  bool isValid = false;
};

struct DomeConfig {
  domeHeader data;
  domeSaveConfigStruct Save;
  domeLoadConfigStruct Load;
};

struct DomeStruct{
  ShutterStruct Shutter;
  DomeConfig config;
};

DomeStruct Dome;
JsonDocument DomeConfig;
/* CONFIG AREA */


#endif