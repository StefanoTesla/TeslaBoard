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
    void connect(String SSID,String Password);
    bool connectionWait();
    void resetIncomingParameters();
    bool checkScannedNetworks();

    //read and write NVS

    AsyncWebServer* _server;
    DNSServer _dnsServer;
    File _fileReader;
    bool _lmsAck = false;
    unsigned long _lms;
    String _incomingSSID ="";
    String _incomingPSW ="";
    bool _incomingDefault = false;
    bool _newIncomingWiFi = false;
    bool _routeInit = false;
    bool _scanInProgress = false;
    bool _dnsServerActive = false;
    bool _skipThisNetwork = false;
    
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
  
    #define MAX_WIFI_NETWORKS 10 // Numero massimo di reti Wi-Fi da salvare

    const char* PREF_NAMESPACE = "wifi_creds";

    typedef struct {
        String ssid;
        String password;
        bool pref;
        bool isValid;
    } WiFiCredentials;

    WiFiCredentials savedNetworks[MAX_WIFI_NETWORKS];

    WiFiCredentials tmpNetwork; //tmp structure used for reordering

    Preferences nvsHandler;

    char ssid_key[15];
    char pass_key[15];
    char pref_key[15];

    void prepareNvsPointer(int i){
      sprintf(ssid_key, "ssid_%d", i);
      sprintf(pass_key, "pass_%d", i);
      sprintf(pref_key, "pref_%d", i);
    }

    bool initNVS(){
      Serial.println("[WiFi Mgr] NVS INIT");

      esp_err_t ret = nvs_flash_init();

      if (ret == ESP_OK) {
          Serial.println("[WiFiMgr] NVS Initialized");
      } else if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
          Serial.printf("[WiFiMgr] NVS Error: %s\n", esp_err_to_name(ret));
          Serial.println("[WiFiMgr] Trying to format it.");
          esp_err_t erase_ret = nvs_flash_erase();
          if (erase_ret == ESP_OK) {
              Serial.println("[WiFiMgr] NVS formatted. Trying to init...");
              ret = nvs_flash_init();
              if (ret == ESP_OK) {
                  Serial.println("[WiFiMgr] NVS Initialized");
                  return true;
              } else {
                  Serial.printf("[WiFiMgr] NVS Critical Error: (%s) after reinitialization\n", esp_err_to_name(ret));
                  Serial.printf("\nUnable to proceed.");
                  while(true) { delay(1000); }
                  return false;
              }
          } else {
                  Serial.printf("[WiFiMgr] NVS Critical Error: (%s) unable to write on it.\n", esp_err_to_name(ret));
                  Serial.printf("\nUnable to proceed.");
              while(true) { delay(1000); } 
              return false;
          }
      } else {
          Serial.printf("[WiFiMgr] NVS Unkwon error (%s)\n", esp_err_to_name(ret));
          Serial.printf("Unable to proceed.\n");
          while(true) { delay(1000); } 
          return false;
      }

      

      if (nvsHandler.begin(PREF_NAMESPACE, false)) {
        nvsHandler.end();
        return true;
      } else {
        Serial.printf("[WiFiMgr] NVS Error (%s) trying to open the namespace.\n", PREF_NAMESPACE);
          while(true) { delay(1000); }
          return false;
      }
    }

    void initWiFiStruct(){
      Serial.println("[WiFiMgr] Stored Wifi:");
      nvsHandler.begin(PREF_NAMESPACE, true);
      for (size_t i = 0; i < MAX_WIFI_NETWORKS; i++)
      {
        prepareNvsPointer(i);

        if(nvsHandler.isKey(ssid_key) && (nvsHandler.getString(ssid_key, "").length() > 0))
          {
          savedNetworks[i].ssid = nvsHandler.getString(ssid_key, "");
          savedNetworks[i].password = nvsHandler.getString(pass_key, "");
          savedNetworks[i].pref = nvsHandler.getBool(pref_key, "");
          savedNetworks[i].isValid = true;
          Serial.printf("%d - %s\n",i, savedNetworks[i].ssid);
          } else {
              savedNetworks[i].isValid = false;
          }
      }
      nvsHandler.end();
    }


    // return the number of valid network stored
    int storedWifi(){
      int validWiFi = 0;
      for (int i = 0; i < MAX_WIFI_NETWORKS; i++)
      {
        if(savedNetworks[i].isValid){
          validWiFi++;
        }
      }
      return validWiFi;
      
    }

    void WriteNVS(){

      WiFiListOrder();
      nvsHandler.begin(PREF_NAMESPACE, false);

      for (int i = 0; i < MAX_WIFI_NETWORKS ; i++)
      {
        prepareNvsPointer(i);

        if(savedNetworks[i].isValid){
          Serial.printf("\n Storing SSID: %s in position %d",savedNetworks[i].ssid,i);
          nvsHandler.putString(ssid_key,savedNetworks[i].ssid);
          nvsHandler.putString(pass_key,savedNetworks[i].password);
          nvsHandler.putBool(pref_key,savedNetworks[i].pref);
        } else {
          
          if(nvsHandler.isKey(ssid_key)){
            Serial.printf("\n Deleting in position %d",i);
            nvsHandler.remove(ssid_key);
            if(nvsHandler.isKey(pass_key)){
              nvsHandler.remove(pass_key);
            }
            if(nvsHandler.isKey(pref_key)){
              nvsHandler.remove(pref_key);
            }
          }
        }
      }
      nvsHandler.end();
      resetIncomingParameters();
    }
};

#endif