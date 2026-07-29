#define ALPACA_PORT 4567

//---- STOP EDITING FROM THIS LINE

#define SW_VERSION "4.0.0"
#include "libraries.h"


AsyncWebServer server(80);
AsyncWebServer alpaca(ALPACA_PORT);
TeslaWiFiManager WiFiManager(&server);
PWMManager pwmMgr;
#include "header.h"


#include "Alpaca/utility.h"
#include "Dome/api.h"
#include "CoverC/api.h"
#include "Switch/api.h"
#include "Board/api.h"
#include "Alpaca/apiManage.h"

BoardModule Board;
DomeModule Dome;
CoverCalibratorModule CoverCalibrator(&pwmMgr);
SwitchModule Switches(&pwmMgr); 

AsyncUDP udp;

#include "Alpaca/discovery.h"

void setup() {
  Serial.begin(115200);
  if(!LittleFS.begin()){
  //  return;
  }

  uint32_t start_time = millis();
  while (!Serial && (millis() - start_time < 4000)) {
    delay(10);
  }

  Serial.setDebugOutput(true);

  Serial.println();
  start_time = millis();

  while (!Serial && (millis() - start_time < 4000)) {
    delay(10);
  }
  Serial.println("ESP32-S3 USB CDC attiva");
  Serial.println("Se leggi qui, stai usando la USB nativa.");
  Serial.println("Begin WIFI");
  WiFiManager.begin();
  Serial.println("Begin WIFI");
  Serial.println("Begin Board");
  Board.begin();
  Serial.println("Begin Board");
  Serial.println("Begin Dome");
  Dome.begin();
  Serial.println("Begin Dome");
  Serial.println("Begin CoverC");
  CoverCalibrator.begin();
  Serial.println("Begin CoverC");
  Serial.println("Begin Switches");
  Switches.begin();
  Serial.println("Begin Switches");
  WiFiManager.setHostName(Board.getIdentifier());
  //start alpaca discovery
  alpacaDiscovery(udp);
  AlpacaManager();
  DomeApi();
  CoverCalibratorApi();
  SwitchApi();
  boardWebServer();

  server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");
  server.serveStatic("/assets/", LittleFS, "/www/assets/").setCacheControl("max-age=604800");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
  
  server.onNotFound([](AsyncWebServerRequest *request) {
    if(WiFiManager.APisRunning()){
      if (request->method() == HTTP_OPTIONS) {
        request->send(200);
      } else {
      request->redirect("/wifi-mgr");
      }
    } else {
      if (request->method() == HTTP_OPTIONS) {
        request->send(200);
      } else {
        request->send(404);
      }
    }
  });
  
  ElegantOTA.begin(&server);
  server.begin();
  alpaca.begin();
  

}

void loop() {

  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());

    if (!rxInProgress) {
      if (c == '<') {
        rxInProgress = true;
        rxIndex = 0;
      }
      continue;
    }

    if (c == '<') {
      rxIndex = 0;
      continue;
    }

    if (c == '>') {
      rxBuffer[rxIndex] = '\0';

      char* sep = strchr(rxBuffer, ':');
      if (!sep) {
        Serial.print("<ERR:BAD_FRAME>");
        rxInProgress = false;
        rxIndex = 0;
        continue;
      }

      *sep = '\0';
      const char* moduleToken = rxBuffer;
      char* payload = sep + 1;
      if (strcmp(moduleToken, "BO") == 0) {
        Board.handlePacket(payload, Serial);
      } else if (strcmp(moduleToken, "DO") == 0) {
        Dome.handlePacket(payload, Serial);
      } else if (strcmp(moduleToken, "CC") == 0) {
        CoverCalibrator.handlePacket(payload, Serial);
      } else if (strcmp(moduleToken, "SW") == 0) {
        Switches.handlePacket(payload, Serial);
      } else {
        Serial.print("<ERR:UNKNOWN_MOD>");
      }

      rxInProgress = false;
      rxIndex = 0;
      continue;
    }

    if (c >= 32 && c <= 126 && rxIndex < sizeof(rxBuffer) - 1) {
      rxBuffer[rxIndex++] = c;
    } else {
      Serial.print("<ERR:BAD_FRAME>");
      rxInProgress = false;
      rxIndex = 0;
    }
  }

  WiFiManager.loop();
  Board.loop();
  Dome.loop();
  CoverCalibrator.loop();
  Switches.loop();
  ElegantOTA.loop();
}

