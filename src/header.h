unsigned int pwmchannles = 1;//channel 0 is reserved to coverCalibrator

/* ALPACA DATA */

struct AlpacaCommonData{
  uint32_t clientTransactionID;
  uint32_t serverTransactionID = 0;
  uint32_t clientID;
  unsigned long LastServerRequest;
  bool boConnect;
  //domeAlpacaParameters dome;
  switchAlpacaParameters switches;
  coverCAlpacaParameters coverC;
};

AlpacaCommonData AlpacaData;

/*END ALPACA DATA */


/** CONFIG STRUCT **/
struct boardSaveConfigStruct{
  bool execute = false;
  bool failed = false;
  bool restartNeeded = false;
};
struct saveConfigStruct{
  domeSaveConfigStruct dome;
  switchSaveConfigStruct switches;
  coverCSaveConfigStruct coverC;
  lidarSaveConfigStruct lidar;
  boardSaveConfigStruct board;
};

struct validConfig{
  domeLoadConfigStruct dome ;
  switchLoadConfigStruct switches;
  coverCLoadConfigStruct coverC;
  lidarLoadConfigStruct lidar;
};

struct coverCalibrationSetting {
  unsigned int pin = 0;
};

struct AlpacaPortsStruct{
  unsigned int remotePort = 32227;
  unsigned int alpacaPort = 4567;
};

struct ConfigStruct{
  saveConfigStruct save;
  validConfig read;
  domeConfig dome;
  coverCalibrationSetting coverC;
  switchConfig switches;
  lidarConfig lidar;
  AlpacaPortsStruct alpacaPort;
};

ConfigStruct Config;
/* END OF CONFIG */



/* GLOBAL VARIABLES */
struct upTimeStruct{
  unsigned long previousMillis = 0;
  unsigned long minutes = 0;
};

struct wifiReconntectionStruct{
  unsigned long intervall = 30000;
  unsigned long lastMillis = 0;
  bool waitToReconnect = false;
};

struct wifiStruct{
  wifiReconntectionStruct reconnection;
  upTimeStruct upTime;
};

struct esp32Struct{
  upTimeStruct upTime;
};

struct globalVariable{
  unsigned long upTimeInterval = 60000;
  esp32Struct esp32;
  wifiStruct wifi;
};

globalVariable Global;


/* END OF GLOBAL */


//common function to validate a pin usage

bool validatePin(int pin, int type){

  // pin from 6 to 11 are connected to integrated spi
  //pin 0 should be low to enter in flash mode, so I avoid to use it.
  if (pin == 0 || (pin >= 6 && pin <= 11)){
    return false;
    Serial.print(F("Pin validation error: cannot be used in any way "));
    Serial.println(pin);
  }

  switch(type){
      case 1: //Digital Output
      case 3: //PWM Output
              #ifdef ESP32S3
              return pin >= 1 ? true : false;
              #else
              return pin >=34 ? false : true;
              #endif
              break;
      case 2: //Digital Input
              return pin >= 4 ? true : false;
              break;
      case 4: //Analog Input
              if(
                #ifdef ESP32S3
                (pin >= 4 && pin <= 18) ||
                (pin >= 20 && pin <= 21) )
                #else
                pin == 2 ||
                pin == 4 ||
                (pin >= 12 && pin <= 15) ||
                (pin >= 25 && pin <= 27) ||
                (pin >= 32 && pin <= 39) )
                #endif
                { return true; } else { return false;}
              break;

      default:
        return false;
        break;
  }
}
