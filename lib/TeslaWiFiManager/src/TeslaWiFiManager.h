#ifndef WIFI_MGR_H
#define WIFI_MGR_H

#include <Arduino.h>
#include "WiFi.h"
#include "LittleFS.h"
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <AsyncJson.h>

/*
Task to do:

- perform async wifi scan
- check if wifi file exist
- read the file and wait for scan completed
- check if exist a saved wifi and try to connect
- on the loop check if wifi still connected, if not try to reconnect to the same wifi
- if connectioon fail for 10 minuter open captive portale (but check if wifi come back)
- made the web page
*/

class TeslaWiFiManager{
  public:
    TeslaWiFiManager(AsyncWebServer *server);
    void init();
    void loop();
    void setStaticIP(IPAddress ip,IPAddress gw,IPAddress sn);
    

  private:
  void startWiFiScan();
  void waitWiFiScanCompleted();
  void readWiFiFile();
  void startCaptivePortal();
  void stopCaptivePortal();
  void serverRouting();
  void serverBegin();
  void serverStop();
  AsyncWebServer* _server;
  DNSServer _dnsServer;
  File _fileReader;
  JsonDocument _json;
  unsigned long _lastMillis;
  bool _routeInit = false;
  bool _scanInProgress = false;
  bool _missingWiFiFile = false; //wifi file doesn't exist
  bool _desWiFiFileError = false; //unable to deserialize wifi file
  bool _dnsServerActive = false;
  unsigned int _cycle =0;
  unsigned int _wifiNetworkFound = 0;

};

#endif