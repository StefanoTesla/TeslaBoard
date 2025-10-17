#define ALPACA_PORT 4567

#define BOARD_IDENTIFIER "TeslaBoard"
#define CC_IDENTIFIER "Cover Calibrator"
#define SWITCH_IDENTIFIER "Switch"


//---- STOP EDITING FROM THIS LINE

#define SW_VERSION "4.0.0"

#include "libraries.h"


AsyncWebServer server(80);
AsyncWebServer alpaca(ALPACA_PORT);
TeslaWiFiManager wi(&server);
PWMManager pwmMgr;
#include "header.h"


#include "Alpaca/common.h"
#include "Alpaca/middleware.h"
#include "Dome/api.h"
#include "CoverC/api.h"
#include "Switch/main.h"
#include "Board/webserver.h"
#include "Board/main.h"
#include "Alpaca/apiManage.h"

DomeModule Dome;
CoverCalibratorModule CoverCalibrator(&pwmMgr);
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
  domeRequestHandler();
  CoverCalibratorApi();
  //switchRequestHandler();
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

  boardLoop();
  Dome.loop();
  CoverCalibrator.loop();
  SwitchLoop();

}
