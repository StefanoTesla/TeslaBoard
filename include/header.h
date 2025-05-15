#define _SW_VERSION_ 399;
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

struct PWMChannelStruct{
  ledcType type;
};

struct GPIOStruct{
  gpioType type;
  ModuleType module;
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
  GPIOStruct gpio[40];
};

globalVariable Global;
/* END OF GLOBAL */

bool pinExist(unsigned int pin){

  if(pin > 39 or pin == 20 or pin == 24 or pin==38 or (pin >=6 and pin <= 11) or (pin >=28 and pin <= 31)){
    return false;
  }

  return true;
}

bool pinUsableAsInput(unsigned int pin){

  if(!pinExist(pin) or pin==1){
    return false;
  }
  return true;
}

bool pinUsableAsOutput(unsigned int pin){

  if(!pinExist(pin) or pin == 3 or pin > 33){

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

int setupLedcChannel(unsigned int channel, ledcType type){

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

int checkForFreeLedChannel(ledcType type){

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

int assignLedChannel(ledcType type){
  int channel = checkForFreeLedChannel(type);
  if (channel < 0 ){
    Serial.println("[ERR] LEDC: error during the search of a free channel.");
    return -1;
  }
  Global.pwm[channel].type = type;
  setupLedcChannel(channel,type);
  return channel;
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
      Serial.print(" | ");
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
