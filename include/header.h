
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



struct oneMinutePulse{
  unsigned long oldMillis;
  bool pulse;
};

struct pulseStruct{
  //oneSecondPulse second;
  oneMinutePulse minute;
};

struct globalVariable{
  unsigned long upTimeInterval = 60000; //one minute, don't change
  //LedCChannels ledcChannel;
  pulseStruct pulse;
  boarcConfigStruct config;

};

globalVariable Global;
/* END OF GLOBAL */

