#ifndef WIFIMGR_H
#define WIFIMGR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ESPAsyncWebServer.h"
#include <Preferences.h>
#include <DNSServer.h>
#include <AsyncJson.h>
#include "WiFi.h"
#include "LittleFS.h"


#define WIFI_SCHEMA_VERSION 1
#define WIFI_SCHEMA_NAME "wifimgr"
#define MAX_CONFIGURED_WIFI 5 //save ram like a tree

class TeslaWiFiManager {
public:
    TeslaWiFiManager(AsyncWebServer *server);

    void begin(); 
    void loop();

    bool APisRunning() { return apRunning; }

   // void getConfiguration(JsonObject dest);

    unsigned long getUptime() { return upTime; }

private:
    void storeConfiguration();
/* functions to handle the configuration */
    bool openNVS(bool readOnly);
    void closeNVS();
    bool initNVS();
    void updateNVS1();

    void web();

    /* function for the scan*/
    enum scanStateEnum{
        SCAN_OFF,
        SCAN_WAIT_START,
        SCAN_SCANNING,
        SCAN_DONE,
        SCAN_TIMEOUT
    };

    scanStateEnum scanStatus = SCAN_OFF;

    bool okToScan = false;
    bool scanInProgress = false;
    bool scanDone = false;
    bool scanTimeOut = false;
    unsigned long scanTimeOutMillis;
    unsigned long scanDelayMillis;
    void scanManager();
    void startWiFiscan();
    void copyWiFiList();
    JsonDocument wifiScanList;

    /* WIFI STATUS */
    bool isWiFiReady;
    bool isWiFiApSta;
    bool isWiFiSta;
    bool connecting = false;


    JsonDocument tmpCfg;

    void connectToWifi(const char* ssid, const char* password);
    bool toBeStored = false;
 
    void storeNewWiFi(String ssid,String password);

    Preferences nvs;
    AsyncWebServer* _server;
    DNSServer _dnsServer;

    enum PrefEnumStatus { CLOSED, OPEN_WRITE, OPEN_READOLNY };
    PrefEnumStatus nvsStatus = CLOSED;
    bool validConfig;
    bool rebootNeeded;
    bool apRunning;
    //uptime data
    unsigned long oneMinMillis = 0;
    unsigned long upTime = 0;



    struct WiFiCredential {
        char ssid[33] ="\0";
        char password[64] = "\0";
    };

    WiFiCredential wifiList[MAX_CONFIGURED_WIFI];
    unsigned int configuredWiFi;

    void handleWiFiEvent(arduino_event_id_t event, arduino_event_info_t info);

    void mainCycle();
    int mainState = 0;
    unsigned long waitChange = 0;


    int APcycle;
    bool incomingWiFi;
    JsonDocument wifiToConnect;
    void APloop();

    bool findMatchingWiFi();
    int lastFound = 0;



    void STAloop();
    int STAConCy;
    void STAConnectionCycle();
};



#endif