// Uncomment the follwing line if your board is a ESP32-S3 (some relay boards)
// #define ESP32S3
#ifdef ESP32S3
  #define _MAXPIN 48
#else
  #define _MAXPIN 40
#endif

#define DOME

#ifdef DOME
/*
Usually I expect to have a gate board, but in the last time I found some guys that handle the motor with two realys, open and close
Only in this case comment the #define GATE_BOARD
#start pin will be open command
#halt pin will be close command
*/
#define GATE_BOARD
  #ifdef GATE_BOARD
    /* un-commnent the following line if your board only activates when the button is released (falling signal)
       if your board activates as soon as the button is pressed (rising signal) then comment the line (//)
    */
    #define ALEKO
  #endif
#endif

// uncomment the following line if you want your dome to check if your onstep mount is parked
#define ONSTEP_MOUNT

//#define SWITCH
//#define COVERC
#define LIDAR

#include <WiFi.h>
#include "AsyncJson.h"
#include "AsyncUDP.h"
#include "HTTPClient.h"
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <stdint.h>
#include "SPIFFS.h"
#include "Dome/domeVariable.h"
#include "Switches/switchVariable.h"
#include "CoverC/coverVariable.h"
#include "Lidar/lidarVariable.h"
#include "header.h"
#include <ElegantOTA.h>

AsyncWebServer server(80);
AsyncWebServer Alpserver(4567);

#include "Alpaca/AlpacaManageFunction.h"

#ifdef DOME
#include "Dome/dome.h"
#endif

#ifdef SWITCH
#include "Switches/switch.h"
#endif

#ifdef COVERC
#include "CoverC/cover.h"
#endif

#ifdef LIDAR
#include "Lidar/lidar.h"
#endif

#include "browserServer.h"
#include "configuration.h"

#include "loop.h"
DNSServer dns;
AsyncUDP udp;

void ServerNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
  Serial.println("404");
  Serial.println(request->url());
}

void setup()
{
  Serial.begin(115200);
  AlpacaData.serverTransactionID = 0;
  /* reading configuration from file */
  if (!SPIFFS.begin()) { Serial.println("An Error has occurred while mounting SPIFFS"); return; }

  #ifdef DOME
  initDomeConfig();
  Serial.println("dome init done");
  #endif

  #ifdef SWITCH
  initSwitchConfig();
  Serial.println("switch init done");
  #endif

  #ifdef COVERC
  initCoverCConfig();
  Serial.println("cover init done");
  #endif

  #ifdef LIDAR
  initLidarConfig();
  Serial.println("LIDAR init done");
  #endif

  Serial.println("Listening for discovery requests...");
  AsyncWiFiManager wifiManager(&server,&dns);
  wifiManager.autoConnect();
  Serial.print("Connect with IP Address: ");
  Serial.println(WiFi.localIP());


  if (udp.listen(32227))
  {
    Serial.println("Listening for discovery requests...");
    udp.onPacket([](AsyncUDPPacket packet) {
      if (packet.length() < 16)
      {
        return;
      }
      //Compare packet to Alpaca Discovery string
      if (strncmp("alpacadiscovery1", (char *)packet.data(), 16) != 0)
      {
        return;
      }
      packet.printf("{\"alpacaport\": 4567}");
    });
  }

  Alpserver.onNotFound(notFound);
  /*** MANAGE AREA ***/

  AlpacaManager();
  #ifdef DOME
  domeServer();
  #endif
  
  #ifdef SWITCH
  switchServer();
  #endif

  #ifdef COVERC
  coverServer();
  #endif

  #ifdef LIDAR
  lidarServer();
  #endif

  browserServer();

  /** END SWITCH SPECIFIC METHODS **/
  Alpserver.begin();
  ElegantOTA.begin(&server);
  server.begin();
  Dome.lastCommunicationMillis = millis();
  Serial.println("setup done");
}

void loop(){
  main_loop();
}
