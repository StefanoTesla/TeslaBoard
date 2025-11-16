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

    unsigned long scanTimeOutMillis;
    unsigned long scanDelayMillis;
    void scanManager();
    void startWiFiscan();
    void copyWiFiList();
    JsonDocument wifiScanList;

    /* WIFI STATUS */
    bool isWiFiReady;
    bool isWiFiSta;
    bool connecting = false;


    JsonDocument tmpCfg;

    void connectToWifi(const char* ssid, const char* password);
    bool toBeStored = false;
 
    void storeNewWiFi(String ssid,String password);
    void removeWiFiById(int id);
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

    enum mainCycleSteps{
        DECISION,
        GOAP_RADIO_OFF,
        GOAP_WAIT_DELAY,
        GOAP_RADIO_ON,
        GOAP_WAIT_AP_READY,
        MAIN_AP_LOOP,
        GOSTA_RADIO_OFF,
        GOSTA_WAIT_DELAY,
        GOSTA_RADIO_ON,
        GOSTA_WAIT_AP_READY,
        MAIN_STA_LOOP,
    };
    mainCycleSteps mainState = DECISION;
    void mainCycle();

    unsigned long waitChange = 0;


    //int APcycle;
    bool incomingWiFi;
    bool connectDirectly;
    bool forceAPRequest;
    unsigned long apRequestMillis;
    bool staDisconnected;
    JsonDocument wifiToConnect;
    enum APCycleEnum {
        AP_INIT,
        AP_WAIT_OPERATION,
        AP_CONNECT_TO_WIFI,
        AP_WAIT_CONNECTION
    };
    APCycleEnum APcycle;
    void APloop();

    bool findMatchingWiFi();
    int lastFound = 0;


    unsigned long connectionTOUTMillis;

    void STAloop();
    
    enum STACycleEnum {
        STA_INIT,
        STA_WAIT_SCAN,
        STA_BEFORE_CONNECT,
        STA_LOOK_FOR_WIFI,
        STA_GO_TO_AP,
        STA_CONNECT_TO_WIFI,
        STA_WAIT_CONNECTION,
        //after connection failed
        STA_DISCONNECT,
        STA_TURN_ON_WIFI,
        STA_WAIT_RADIO_ON
    };

    bool gpio0pressed;
    unsigned long waitOneSecondMillis;
    STACycleEnum STAConCy;
    void STAConnectionCycle();
};



#endif