#ifndef DOME_VARIABLE
#define DOME_VARIABLE


/* BASE AREA */

enum ShInEnum {
  ShInNoOne,
  ShOnlyClose,
  ShOnlyOpen,
  ShInAll
};
enum ShStEnum {
  ShOpen,
  ShClose,
  ShOpening,
  ShClosing,
  ShError
};
enum ShCmdEnum {
  Idle,
  CmdOpen,
  CmdClose,
  CmdHalt
};

struct DomeStruct{
  ShCmdEnum ShutterCommand;
  ShStEnum ShutterState;
  ShInEnum ShutterInputState;
  bool ShutterManualInputState;
  int Cycle;
  bool Safe;
  bool MoveRetry;
  unsigned int LastDomeCommand =0;
  unsigned long lastCommunicationMillis;
};

DomeStruct Dome;

/* ALPACA AREA */

struct domeAlpacaStruct{
  ShCmdEnum ShutterCommand;
  ShStEnum ShutterState;
  ShInEnum ShutterInputState;
};

/* CONFIG AREA */

struct domeConfig{
  unsigned int pinStart;
  unsigned int pinHalt;
  unsigned int pinOpen;
  unsigned int pinClose;
  unsigned int pinManual;
  unsigned int movingTimeOut = 20;
  bool enAutoClose;
  unsigned int autoCloseTimeOut = 20;
  String mountIP;
  unsigned int mountPort;
  bool secCheck;
  bool secStrict;
};

struct domeSaveConfigStruct{
  bool execute = false;
  bool failed = false;
  bool restartNeeded = false;
};

struct domeLoadConfigStruct{
  bool isValid = false;
};

#endif