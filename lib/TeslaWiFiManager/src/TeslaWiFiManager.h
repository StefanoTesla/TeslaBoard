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
    void storeWiFiConnection();
    void WiFiListOrder();
    void deleteWiFiConnection(String ssid);
    bool checkWiFiFile();
    void startCaptivePortal();
    void stopCaptivePortal();
    void serverRouting();
    void serverBegin();
    void serverStop();
    void connect(String SSID,String Password);
    bool connectionWait();

    AsyncWebServer* _server;
    DNSServer _dnsServer;
    File _fileReader;
    JsonDocument _json;
    JsonArray _wifiList;
    JsonObject _forObj;
    bool _lmsAck = false;
    unsigned long _lms;
    String _incomingSSID ="";
    String _incomingPSW ="";
    bool _incomingDefault = false;
    bool _newIncomingWiFi = false;
    bool _routeInit = false;
    bool _scanInProgress = false;
    bool _dnsServerActive = false;
    unsigned int _cycle =0;
    unsigned int _wifiNetworkFound = 0;

};

#endif