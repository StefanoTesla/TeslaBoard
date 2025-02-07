/* input structure */
struct InputStructure {
  unsigned int pin;
  bool type; //0=NO 1=NC
};

/* ALPACA DATA */

struct AlpacaCommonData{
  uint32_t clientTransactionID;
  uint32_t serverTransactionID = 0;
  uint32_t clientID;
};

AlpacaCommonData AlpacaData;

/*END ALPACA DATA */


/** CONFIG STRUCT **/
struct boardSaveConfigStruct{
  bool execute = false;
  bool failed = false;
  bool restartNeeded = false;
};

/* END OF CONFIG */

/* GLOBAL VARIABLES */
struct upTimeStruct{
  unsigned long minutes = 0;
};

struct wifiReconntectionStruct{
  unsigned long intervall = 10;
  unsigned long delaySeconds = 0;
  bool waitToReconnect = false;
};

struct wifiIpStruct{
  bool enable = false;
  unsigned int ip[4];
  unsigned int sub[4];
  unsigned int gw[4];
};

struct wifiStruct{
  wifiReconntectionStruct reconnection;
  upTimeStruct upTime;
  wifiIpStruct ip;
};

struct esp32Struct{
  upTimeStruct upTime;
};

struct localeStruct{
  String locale = "en";
};

struct RebootRequest{
  unsigned long lastMillis = 0;
  bool rebootRequest;
};

struct boarcConfigStruct{
  RebootRequest reboot;
  wifiStruct wifi;
  esp32Struct esp32;
  localeStruct language;
  boardSaveConfigStruct save;
};

enum ledcType{
  notAssigned,
  pwm,
  servo
};

struct PWMChannelStruct{
  ledcType type;
};

struct oneMinutePulse{
  unsigned long oldMillis;
  bool pulse;
};

struct oneSecondPulse{
  unsigned long oldMillis;
  bool pulse;
};

struct pulseStruct{
  oneSecondPulse second;
  oneMinutePulse minute;
};


struct globalVariable{
  unsigned long actualMillis;
  unsigned long upTimeInterval = 60000; //one minute, don't change
  pulseStruct pulse;
  boarcConfigStruct config;
  PWMChannelStruct pwm[16];
};

globalVariable Global;
/* END OF GLOBAL */


enum LogMachine{
  board,
  dome,
  coverc,
  Switches
};

enum LogType{
  lErr,
  lInfo,
  lDebug
};

bool pinExist(unsigned int pin){
  if(pin > 39 or pin == 20 or pin == 24 or pin==38){
    return false;
  }

  if(pin >=6 and pin <= 11){
    return false;
  }


  if(pin >=28 and pin <= 31){
    return false;
  }

  return true;
}
bool pinUsableAsInput(unsigned int pin){
  if(!pinExist(pin)){
    return false;
  }

  if(pin==1){
    return false;
  }

  return true;

}
bool pinUsableAsOutput(unsigned int pin){

  if(!pinExist(pin)){
    return false;
  }

  if(pin == 3){
    return false;
  }

  if(pin > 33){
    return false;
  }

  return true;

}
bool pinUsableAsAnalogInput(unsigned int pin){
  if(pin <32){
    return false;
  }
  return true;
}
bool pinUsableAsAnalogOutput(unsigned int pin){
  if(pin < 25 or pin > 25){
    return false;
  }
  return true;
}


bool usableLedChannel(unsigned int channel,ledcType type){

  if(type == notAssigned){
    Serial.println("[ERR] LEDC: Invalid type");
    return false;
  }

  if(channel > 15){
    Serial.println("[ERR] LEDC: Invalid channel number, maximum is 15");
    return false;
  }

  if(Global.pwm[channel].type != notAssigned){
    return false;
  }

  int x = 0;
  int otherChannel=0;

  float ch = channel / 2.0;
  x = ch;
  float modulo = ch - x;

  if(modulo > 0.0){ 
    otherChannel = channel - 1; 
  } else {
    otherChannel = channel + 1; 
  }

  if(Global.pwm[otherChannel].type != notAssigned && Global.pwm[otherChannel].type != type){
    return false;
  }

  return true;

}

unsigned int setupLedcChannel(unsigned int channel, ledcType type){

  switch (type)
  {
  case pwm:
    if(ledcSetup(channel, 19531, 12) > 0){
      return channel;
    } else {
      Serial.println("[ERR] INIT: Error during pwm cahnnel setup");
    }
    break;
  case servo:
    if(ledcSetup(channel, 50, 12) > 0){
      return channel;
    } else {
      Serial.println("[ERR] INIT: Error during pwm cahnnel setup");
    }
    break;
  
  default:
      return -1;
    break;
  }

  return -1;
}

unsigned int checkForFreeLedChannel(ledcType type){

  if (type == notAssigned){
    return -1;
  }

  if(type == pwm){
    for (int i = 0; i < 16; i++)
    {
      if(usableLedChannel(i,type)){ return i;}
    }
  } else if (type == servo) {

    //since servo are at 50hz we give the precedence to low speed timer
    for (int i = 8; i < 16; i++)
    {
      if(usableLedChannel(i,type)){ return i;}
    }

    for (int i = 0; i < 8; i++)
    {
      if(usableLedChannel(i,type)){ return i;}
    }

  }
  return -1;
}

unsigned int assignLedChannel(ledcType type){
  unsigned int channel = checkForFreeLedChannel(type);
  if (channel < 0 ){
    Serial.println("[ERR] LEDC: error during the search of a free channel.");
    return -1;
  }
  Global.pwm[channel].type = type;
  setupLedcChannel(channel,type);
  return channel;

    
  return -1;


}

void printLEDChannelStatus(){

  Serial.println("");
  Serial.println("ch | type       |");
  Serial.println("------------------");
  for (int i = 0; i < 16; i++)
  {
    if(i<10){
      Serial.print(i);
      Serial.print("  |");
      Serial.print(" ");
      switch (Global.pwm[i].type)
      {
      case notAssigned:
        Serial.println("unassigned | ");
        break;
      case pwm:
        Serial.println("pwm        | ");
        break;
      case servo:
        Serial.println("servo      | ");
        break;
      
      default:
        break;
      }

    } else {

      Serial.print(i);
      Serial.print(" |");
      Serial.print(" ");
      switch (Global.pwm[i].type)
      {
      case notAssigned:
        Serial.println("unassigned |");
        break;
      case pwm:
        Serial.println("pwm |");
        break;
      case servo:
        Serial.println("servo |");
        break;
      
      default:
        break;
      }

    }
  }
  
}

void logMessage(LogMachine machine, LogType type, const char* message) {
    char buffer[200];
    snprintf(buffer, sizeof(buffer), "%d#%d#%s", type, machine, message);
    ws.textAll(buffer);
}

void logMessageFormatted(LogMachine machine, LogType type, const char* format, ...) {
    char message[256]; // Buffer per il messaggio finale
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    logMessage(machine, type, message); // Chiama la tua funzione logMessage
}
