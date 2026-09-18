#ifndef WIFIMGR_H
#define WIFIMGR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ESPAsyncWebServer.h"
#include <DNSServer.h>
#include <AsyncJson.h>
#include "WiFi.h"
#include "LittleFS.h"
#include <NVSManager.h>


#define WIFI_SCHEMA_VERSION 1
#define WIFI_SCHEMA_NAME "wifimgr"
#define MAX_CONFIGURED_WIFI 5 //save ram like a tree

class TeslaWiFiManager {
public:
    TeslaWiFiManager(AsyncWebServer *server);

    void begin();
    void setHostName(String inHostName){ hostName = inHostName;};
    void loop();

    bool APisRunning() { return apRunning; }

   // void getConfiguration(JsonObject dest);

    unsigned long getUptime() { return upTime; }

private:
    void storeConfiguration();
/* functions to handle the configuration */
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

    unsigned long scanTimeOutMillis = 0;
    unsigned long scanDelayMillis = 0;
    void scanManager();
    void startWiFiscan();
    void copyWiFiList();
    JsonDocument wifiScanList;
    String hostName;
    /* WIFI STATUS */
    bool isWiFiSta = false;

    JsonDocument tmpCfg;

    void connectToWifi(const char* ssid, const char* password);
    bool toBeStored = false;
 
    void storeNewWiFi(String ssid,String password);
    void removeWiFiById(int id);
    AsyncWebServer* _server;
    DNSServer _dnsServer;

    bool apRunning = false;
    bool gotIP = false;
    //uptime data
    unsigned long oneMinMillis = 0;
    unsigned long upTime = 0;



    struct WiFiCredential {
        char ssid[33] ="\0";
        char password[64] = "\0";
    };

    WiFiCredential wifiList[MAX_CONFIGURED_WIFI];
    unsigned int configuredWiFi = 0;

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
    bool incomingWiFi = false;
    bool connectDirectly = false;
    bool forceAPRequest = false;
    unsigned long apRequestMillis = 0;
    bool staDisconnected = false;
    JsonDocument wifiToConnect;
    enum APCycleEnum {
        AP_INIT,
        AP_WAIT_OPERATION,
        AP_CONNECT_TO_WIFI,
        AP_WAIT_CONNECTION
    };
    APCycleEnum APcycle = AP_INIT;
    void APloop();

    bool findMatchingWiFi();
    int lastFound = 0;


    unsigned long connectionTOUTMillis = 0;

    void STAloop();
    
    enum STACycleEnum {
        STA_INIT,
        STA_WAIT_SCAN,
        STA_BEFORE_CONNECT,
        STA_LOOK_FOR_WIFI,
        STA_GO_TO_AP,
        STA_CONNECT_TO_WIFI,
        STA_WAIT_CONNECTION,
        STA_CONNECTED,
        //after connection failed
        STA_DISCONNECT,
        STA_TURN_ON_WIFI,
        STA_WAIT_RADIO_ON
    };

    bool gpio0pressed = 0;
    STACycleEnum STAConCy = STA_INIT;
    void STAConnectionCycle();
};



#endif
