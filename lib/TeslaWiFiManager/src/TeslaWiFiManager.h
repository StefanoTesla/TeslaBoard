#ifndef WIFI_MGR_H
#define WIFI_MGR_H

#include <Arduino.h>
#include <Preferences.h> 
#include <nvs_flash.h> 
#include <string.h> 
#include "WiFi.h"
#include "LittleFS.h"
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <AsyncJson.h>
#include "WiFi/TeslaWiFiSta.h"
#include "WiFi/TeslaWiFiAp.h"

/*
Task to do:

- on the loop check if wifi still connected, if not try to reconnect to the same wifi
- if connectioon fail for 10 minuter open captive portale (but check if wifi come back)
*/

class TeslaWiFiManager{
  public:
    TeslaWiFiManager(AsyncWebServer *server);
    void init();
    void loop();
    void setStaticIP(IPAddress ip,IPAddress gw,IPAddress sn);
    

  private:

    typedef struct {
        String ssid;
        String password;
        bool pref;
        bool isValid;
    } WiFiCredentials;


    void startWiFiScan();
    void waitWiFiScanCompleted();
    bool asyncWaitWiFiScan();
    void storeWiFiConnection();
    void WiFiListOrder();
    void deleteWiFiConnection(String ssid);
    void APstart();
    void APstop();
    void APLoop();
    void serverRouting();
    void serverBegin();
    void serverStop();
    void connect(WiFiCredentials network);
    bool connectionWait();
    void resetWiFiCredential(WiFiCredentials network);
    bool checkScannedNetworks();
    bool initNVS(); //read all the wifi from NVS memory
    void prepareNvsPointer(int i); //prepare the NVS block name
    int storedWifi(); //read the number of WiFi in memory
    void writeNVS(); //write all the wifi from NVS memory
    void printIP(); //print the IP addres

    void PortalCycle();
    //read and write NVS

    AsyncWebServer* _server;
    DNSServer _dnsServer;
    File _fileReader;
    TeslaWiFiAP ap;
    bool _lmsAck = false;
    unsigned long _lms;
    unsigned long _reconnectTimeOut;
    bool _newIncomingWiFi = false;
    bool _routeInit = false;
    bool _scanInProgress = false;
    bool _dnsServerActive = false;
    bool _skipThisNetwork = false;
    bool _connectionTentative = false;
    
    unsigned int _StoredWiFiPointer = 0;
    unsigned int _SSIDWiFiPointer = 0;
    int _wifiNetworkFound = 0;

    
    enum initCycle {
      INIT,
      CONNECT_TO_STORED_WIFI,
      START_SCAN_BEFORE_AP,
      START_SCAN,
      WAIT_FOR_SCAN,
      BEFORE_START_AP,
      AP_START,
      WAIT_AP_RUNNING,
      AP_LOOP,
      AP_STOP,
      WAIT_AP_STOP,
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
      L_WAIT_CONNECTION_TO_STORED_WIFI,
      L_CONNECTION_RETRY,
      L_CONNECTION_CHECK,
    };

    loopCycle _loopCycle = L_LOOP;
  
    #define MAX_WIFI_NETWORKS 10 // Numero massimo di reti Wi-Fi da salvare

    const char* PREF_NAMESPACE = "wifi_creds";

    WiFiCredentials _savedNetworks[MAX_WIFI_NETWORKS];

    WiFiCredentials _tmpNetwork; //tmp structure used for reordering
    WiFiCredentials _toConnectNetwork; //tmp structure used for reordering

    Preferences _nvsHandler;

    char ssid_key[15];
    char pass_key[15];
    char pref_key[15];


  };
#endif