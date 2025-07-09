#define ALPACA_PORT 4567

#define BOARD_IDENTIFIER "teslaboard"

#define DOME
#define DOME_IDENTIFIER "Dome"

#define COVER_CALIBRATOR 
#define CC_IDENTIFIER "Cover Calibrator"

#define SWITCH
#define SWITCH_IDENTIFIER "Switch"

#include "libraries.h"

AsyncWebServer server(80);
AsyncWebServer alpaca(ALPACA_PORT);
TeslaWiFiManager wi(&server);

#include "header.h"

#ifdef DOME
#include "Dome/variables.h"
#endif
#ifdef COVER_CALIBRATOR
#include "CoverC/variables.h"
#endif
#ifdef SWITCH
#include "Switch/variables.h"
#endif

#include "Alpaca/middleware.h"
#include "Alpaca/common.h"
#include "Alpaca/apiManage.h"

#ifdef DOME
#include "Dome/main.h"
#endif
#ifdef COVER_CALIBRATOR
#include "CoverC/main.h"
#endif
#ifdef SWITCH
#include "Switch/main.h"
#endif

#include "Board/webserver.h"
#include "Board/main.h"

DNSServer dns;
AsyncUDP udp;

#include "Alpaca/discovery.h"


void setup() {
  Serial.begin(115200);
  
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  startupTask();
  WiFi.setHostname(BOARD_IDENTIFIER);
  
  wi.init();

  //start alpaca discovery
  alpacaDiscovery(udp);
  AlpacaManager();
  #ifdef DOME
    domeRequestHandler();
  #endif
  #ifdef COVER_CALIBRATOR
    coverCalibratorRequestHandler();
  #endif
  #ifdef SWITCH
    switchRequestHandler();
  #endif
  
  boardWebServer();

  server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");
  server.serveStatic("/assets/", LittleFS, "/www/assets/").setCacheControl("max-age=604800");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
  
  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    }
  });
  
  server.begin();
  alpaca.begin();
  ElegantOTA.begin(&server);
}

void loop() {
  Global.actualMillis = millis();
  
  boardLoop();
  #ifdef DOME
  domeLoop();
  #endif
  #ifdef COVER_CALIBRATOR
  coverCalibratorLoop();
  #endif
  #ifdef SWITCH
  SwitchLoop();
  #endif
}
