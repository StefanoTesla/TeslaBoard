#define ALPACA_PORT 4567

#define BOARD_IDENTIFIER "TeslaBoard"
#define DOME_IDENTIFIER "Dome"
#define CC_IDENTIFIER "Cover Calibrator"
#define SWITCH_IDENTIFIER "Switch"


//---- STOP EDITING FROM THIS LINE

#define SW_VERSION "4.0.0"


#include "libraries.h"

AsyncWebServer server(80);
AsyncWebServer alpaca(ALPACA_PORT);
TeslaWiFiManager wi(&server);

#include "header.h"

#include "Dome/variables.h"
#include "CoverC/variables.h"
#include "Switch/variables.h"
#include "Alpaca/middleware.h"
#include "Alpaca/common.h"
#include "Alpaca/apiManage.h"
#include "Dome/main.h"
#include "CoverC/main.h"
#include "Switch/main.h"
#include "Board/webserver.h"
#include "Board/main.h"

DNSServer dns;
AsyncUDP udp;

#include "Alpaca/discovery.h"


void setup() {
  Serial.begin(115200);

  startupTask();
  WiFi.setHostname(BOARD_IDENTIFIER);
  
  wi.init();

  //start alpaca discovery
  alpacaDiscovery(udp);
  AlpacaManager();
  domeRequestHandler();
  coverCalibratorRequestHandler();
  switchRequestHandler();
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
  domeLoop();
  coverCalibratorLoop();
  SwitchLoop();
}
