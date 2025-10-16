
/* ALPACA DATA */

struct AlpacaCommonData{
  uint32_t serverTransactionID = 0;
};

AlpacaCommonData AlpacaData;

/*END ALPACA DATA */


/** CONFIG STRUCT **/
struct boardSaveConfigStruct{
  bool execute = false;
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

enum gpioType{
  notUsed,
  input,
  output,
  pwmOut,
  servoOut
};

enum ModuleType{
  noModule,
  domeModule,
  coverCModule,
  switchModule
};

struct GPIOStruct{
  gpioType type;
  ModuleType module;
};

struct LedCChannels{
  int fastTimer[8];
  int slowTimer[8];
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
  unsigned long upTimeInterval = 60000; //one minute, don't change
  LedCChannels ledcChannel;
  pulseStruct pulse;
  boarcConfigStruct config;
  GPIOStruct gpio[40];
};

globalVariable Global;
/* END OF GLOBAL */

bool pinExist(unsigned int pin){
  if(pin == 1 or pin == 3 or (pin >=6 and pin <=11) or pin == 20 or pin == 24 or (pin >=28 and pin <= 31) or pin == 37 or pin == 38 or pin > 39){
    return false;
  }

  return true;
}

bool pinUsableAsInput(unsigned int pin){

  if(!pinExist(pin) or pin==0 or pin==12){
    return false;
  }
  return true;
}

bool pinUsableAsOutput(unsigned int pin){

  if(!pinExist(pin) or pin > 33){

    return false;
  }
  return true;
}

bool pinUsableAsAnalogInput(unsigned int pin){
  if(!pinExist(pin) and pin <33){
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

int validateJsonInput(JsonObject json){
/*
return code table:
1 validation is ok
-1: pin is not unsigned integer
-10: pin is not usable as input
-2: dOn is not unsigned integer
-3: dOff is not unsigned integer
-4: invert is not unsigned integer
-400: invert is out of range
*/

  if(!json["pin"].is<unsigned int>()){
      return -1;
  } else {
    if(!pinUsableAsInput(json["pin"].as<unsigned int>())){
        return -10;
    }
  }
  if(!json["dOn"].is<unsigned int>()){
      return -2;
  }
  if(!json["dOff"].is<unsigned int>()){
      return -3;
  }
  if(!json["invert"].is<unsigned int>()){
      return -4;
    } else {
      int i;
      i = json["invert"].as<unsigned int>();
      if(i<0 || i>1){
        return -400;
      }
  }

  return 1;
  
}

int validateJsonOutput(JsonObject json){
/*
return code table:
0: wrong type
1 validation is ok
-1: pin is not unsigned integer
-10: pin is not usable as output
-4: invert is not unsigned integer
-400: invert is out of range
*/

  if(!json["pin"].is<unsigned int>()){
      return -1;
    } else {
      if(!pinUsableAsOutput(json["pin"].as<unsigned int>())){
        return -10;
      }
    }

  if(!json["invert"].is<unsigned int>()){
      return -4;
    } else {
      int i;
      i = json["invert"].as<unsigned int>();
      if(i<0 || i>1){
        return -400;
      }
    }

  return 1;
  
}

int validateJsonPwm(JsonObject json){
/*
return code table:
1 validation is ok
-1: pin is not unsigned integer
-10: pin is not usable as output
*/

  if(!json["pin"].is<unsigned int>()){
      return -1;
    } else {
      if(!pinUsableAsOutput(json["pin"].as<unsigned int>())){
        return -10;
      }
    }

  return 1;
  
}

int validateJsonServo(JsonObject json){
/*
return code table:
1 validation is ok
-1: pin is not unsigned integer
-10: pin is not asable as output
-5:maxServo not unsigned integer
-500: maxServo is out of range
-6:openDeg not unsigned integer
-600: openDeg is out of range
-601: openDeg is bigger than maxDeg
-7:closeDeg not unsigned integer
-700: closeDeg is out of range
-701: closeDeg is bigger than maxDeg
-8:movingTime not unsigned integer
-10: type is not unsigned integer
-1010: wrong type passed
*/

  int maxDeg = 0;
  int openDeg = 0;
  int closeDeg = 0;

  if(!json["pin"].is<unsigned int>()){
      return -1;
    } else {
      if(!pinUsableAsOutput(json["pin"].as<unsigned int>())){
        return -10;
      }
  }

  if(!json["maxDeg"].is<unsigned int>()){
      return -5;
    } else {
      maxDeg = json["maxDeg"].as<unsigned int>();
      if(maxDeg>360){
        return -500;
      }
  }
  if(!json["openDeg"].is<unsigned int>()){
      return -6;
    } else {
      openDeg = json["openDeg"].as<unsigned int>();
      if(openDeg>360){
        return -600;
      }
      if(openDeg>maxDeg){
        return -601;
      }
  }
  
  if(!json["closeDeg"].is<unsigned int>()){
      return -7;
    } else {
      closeDeg = json["closeDeg"].as<unsigned int>();
      if(closeDeg>360){
        return -700;
      }
      if(closeDeg>maxDeg){
        return -701;
      }
  }

  if(!json["movTime"].is<unsigned int>()){
      return -8;
  }

  return 1;
  
}

const char* retValTranslate(int retVal) {

  switch (retVal) {
    case -1:
      return "IOPinNotInt";
    case -10:
      return "IOPinNotUsable";
      break;
    case -2:
      return "IOdOnNotInt";
      break;
    case -3:
      return "IOdOffNotInt";
    case -4:
      return "IOInvertNotInt";
    case -400:
      return "IOInvertOutRange";
    case -5:
      return "IOMaxDNotInt";
    case -500:
      return "IOMaxDOutRange";
    case -6:
      return "IOOpDNotInt";
    case -600:
      return "IOOpDBig";
    case -7:
      return "IOClDNotInt";
    case -700:
      return "IOClDBig";
    case -8:
      return "IOMovTimeNotInt";
    // Aggiungi altri casi se necessario
    default:
      return "IOGenErr";
  }
}


//Utilities to copy GPIO faster avoiding keys injection

void copyInputJson(JsonObject input,JsonObject out){
  out["pin"] = input["pin"].as<unsigned int>();
  out["dOn"] = input["dOn"].as<unsigned long>();
  out["dOff"] = input["dOff"].as<unsigned long>();
  out["invert"] = input["invert"].as<unsigned int>();  
}

void copyOutputJson(JsonObject input,JsonObject out){
  out["pin"] = input["pin"].as<unsigned int>();
  out["invert"] = input["invert"].as<unsigned int>();
}

void copyPWMJson(JsonObject input, JsonObject out){
  out["pin"] = input["pin"].as<unsigned int>();
}

void copyServoJson(JsonObject input, JsonObject out){
  out["pin"] = input["pin"].as<unsigned int>();
  out["maxDeg"] = input["maxDeg"].as<unsigned int>();
  out["openDeg"] = input["openDeg"].as<unsigned int>();
  out["closeDeg"] = input["closeDeg"].as<unsigned int>();
  out["movTime"] = input["movTime"].as<unsigned long>();
}


//Utilities to find a free ledc channel

int findLedCChannel(bool preferSlow = false){
  int retVal = -1;

  if(preferSlow){
    for (int i = 0; i < 8; i++)
      {
        if(Global.ledcChannel.slowTimer[i]==0){
          Global.ledcChannel.slowTimer[i]= 1;
          retVal = i;
          return retVal + 8;
        }
      }    
  }

  //search for a fast channel
  for (int i = 0; i < 8; i++)
  {
    if(Global.ledcChannel.fastTimer[i]==0){
      Global.ledcChannel.fastTimer[i]= 1;
      retVal = i;
      return retVal;
    }
  }
  
  //search into slow time if a fast one was no found
  if(!preferSlow){
  for (int i = 0; i < 8; i++)
    {
      if(Global.ledcChannel.slowTimer[i]==0){
        Global.ledcChannel.slowTimer[i]= 1;
        retVal = i;
        return retVal + 8;
      }
    } 

  }

  return retVal; //should be -1 at this point
}

