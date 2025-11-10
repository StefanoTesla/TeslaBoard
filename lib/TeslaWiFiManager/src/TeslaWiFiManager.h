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
#define MAX_CONFIGURED_WIFI 10

class TeslaWiFiManager {
public:
    TeslaWiFiManager(AsyncWebServer *server);

    void begin(); 
    void loop();

    bool APisRunning() { return apRunning; }

    void getConfiguration(JsonObject dest);

    

    unsigned long getUptime() { return upTime; }

private:
    void storeConfiguration();
/* functions to handle the configuration */
    bool openNVS(bool readOnly);
    void closeNVS();
    bool initNVS();
    void updateNVS1();

    void web();

    void scanManager();
    JsonDocument wifiScanList;
    void startWiFiscan();
    void copyWiFiList();

    

    JsonDocument tmpCfg;

    JsonDocument wifiToConnect;
    void connectToWifi();
    bool toBeStored = false;
 
    void storeNewWiFi();
    void deleteWiFi(int id);


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

    bool okToScan;
    bool scanInProgress;
    unsigned long scanTimeOutMillis;
    unsigned long scanDelayMillis;

    struct WiFiCredential {
        char ssid[33] ="\0";
        char password[64] = "\0";
        bool preferred = false;
    };

    WiFiCredential wifiList[MAX_CONFIGURED_WIFI];
    unsigned int configuredWiFi;


    void handleWiFiEvent(arduino_event_id_t event, arduino_event_info_t info);
};

#endif