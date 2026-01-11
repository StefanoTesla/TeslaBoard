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

void initNVS(){
      esp_err_t ret = nvs_flash_init();

    if (ret == ESP_OK) {

    } else if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        esp_err_t erase_ret = nvs_flash_erase();
        if (erase_ret == ESP_OK) {
            ret = nvs_flash_init();
            if (ret == ESP_OK) {
                return;
            }
        }
    }
  }


void setup() {
  initNVS();
  Serial.begin(115200);
  if(!LittleFS.begin()){
  //  return;
  }
 
  WiFiManager.begin();
  Board.begin();
  Dome.begin();
  CoverCalibrator.begin();
  Switches.begin();
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
  WiFiManager.loop();
  Board.loop();
  Dome.loop();
  CoverCalibrator.loop();
  Switches.loop();
  ElegantOTA.loop();
}

