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
    bool asyncWaitWiFiScan();
    void readWiFiFile();
    void writeWiFiFile();
    void storeWiFiConnection();
    void WiFiListOrder();
    void deleteWiFiConnection(String ssid);
    bool checkWiFiFile();
    void APstart();
    void APstop();
    void APLoop();
    void serverRouting();
    void serverBegin();
    void serverStop();
    void connect(String SSID,String Password);
    bool connectionWait();
    void resetIncomingParameters();
    bool checkScannedNetworks();

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
    String _SSIDtoConnect ="";
    String _PSWtoConnect ="";
    bool _incomingDefault = false;
    bool _newIncomingWiFi = false;
    bool _routeInit = false;
    bool _scanInProgress = false;
    bool _dnsServerActive = false;
    bool _skipThisNetwork = false;
    
    unsigned int _retry = 0;
    unsigned int _storedWifi = 0;
    unsigned int _StoredWiFiPointer = 0;
    unsigned int _SSIDWiFiPointer = 0;
    int _wifiNetworkFound = 0;

    enum initCycle {
      INIT,
      STORED_ORDERING,
      CONNECT_TO_STORED_WIFI,
      START_SCAN_BEFORE_AP,
      START_SCAN,
      WAIT_FOR_SCAN,
      BEFORE_START_AP,
      AP_START,
      WAIT_AP_RUNNING,
      AP_LOOP,
      AP_STOP,
      STORE_INCOMING_WIFI,
      CONNECT_TO_WIFI,
      WAIT_FOR_CONNECTION,
      END
    };

    initCycle _cycle = INIT;

    enum loopCycle {
      L_LOOP,
      L_DISCONNECT_WIFI,
      L_SHUTDOWN_WIFI,
      L_BACK_TO_STA_MODE,
      L_WAIT_BEFORE_SCAN,
      L_START_SCAN,
      L_WAIT_SCAN,
      L_CHECK_CONFIGURED_WIFI,
      L_WAIT_FOR_A_NEW_RESCAN,
      L_START_CONNECTION_TO_CONFIGUERD_WIFI,
      L_WAIT_CONNECTION_TO_STORED_WIFI
    };

    loopCycle _loopCycle = L_LOOP;
};

#endif