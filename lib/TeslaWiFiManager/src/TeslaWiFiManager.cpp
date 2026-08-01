#include "TeslaWiFiManager.h"
#include "esp_log.h"
#define LOG_TAG "WiFiMgr"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

TeslaWiFiManager::TeslaWiFiManager(AsyncWebServer *server) : _server(server) {}

#pragma region CONFIGURATION
/* initialize the board */
void TeslaWiFiManager::begin() {
    LOGV("Setting up GPIO 0 for force captive portal");
    pinMode(0, INPUT_PULLUP);
    LOGI("Loading configuration");

    if (!NvsManager::getInstance().openNVS(true,WIFI_SCHEMA_NAME)) {
        LOGE("Error loading wifi nvs partition in read only, trying to format it");
        if (!initNVS()) {
        LOGE("NVS INITIALIZATION FAILED");
        return;
        }
    }

    LOGV("Checking the schema version");
    // if I'm here NVS is surelly working, no more check...
    NvsManager::getInstance().openNVS(true,WIFI_SCHEMA_NAME);
    int schemaVersion = !NvsManager::getInstance().getInt("schema", 0);
    LOGD("schema version is: %d", schemaVersion);

    if (schemaVersion < WIFI_SCHEMA_VERSION) {
        LOGW("Schema version: %d, new version: %d", schemaVersion, WIFI_SCHEMA_VERSION);
        switch (schemaVersion) {
        case 0:
        LOGI("upgrading from 0 to 1");
        updateNVS1();
        break;

        default:
        break;
        }
    }

    NvsManager::getInstance().openNVS(true,WIFI_SCHEMA_NAME);
    

    LOGV("%d configured WiFi",configuredWiFi);
    for (int i = 0; i < MAX_CONFIGURED_WIFI ; i++){
      tmpCfg.clear();
      char key[10];
      snprintf(key, sizeof(key), "wifi%d", i);
      String wifiX= NvsManager::getInstance().getString(key,"{}");
      deserializeJson(tmpCfg,wifiX);
      
      if(tmpCfg.size() != 0){
        const char* ssid = tmpCfg["ssid"] | "";
        const char* password = tmpCfg["psw"] | "";
        strlcpy(wifiList[i].ssid, ssid, sizeof(wifiList[i].ssid));
        strlcpy(wifiList[i].password, password, sizeof(wifiList[i].password));
        LOGV("SSID: %s PASSWORD: %s", wifiList[i].ssid, wifiList[i].password);
        configuredWiFi++;
      }
    }

    NvsManager::getInstance().closeNVS();

    web();

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info){
      this->handleWiFiEvent(event, info);
    });

    WiFi.setHostname(hostName.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(false);

}

bool TeslaWiFiManager::initNVS() {

  LOGW("Board nvs area will be formatted");
  if (!NvsManager::getInstance().openNVS(false,WIFI_SCHEMA_NAME)) {
    LOGE("Unable to open the namespace with write rights, initialization failed");
    return false;
  }
  LOGI("namespace open or created, writing default parameters");

  NvsManager::getInstance().putInt("schema", 0);
  NvsManager::getInstance().putInt("cfgwifi", 0);
  
  NvsManager::getInstance().closeNVS();
  return true;
}

void TeslaWiFiManager::updateNVS1() {
  // this is the first schema, don't check if something already exist.
  NvsManager::getInstance().openNVS(false,WIFI_SCHEMA_NAME);

  NvsManager::getInstance().putInt("schema", 1);

  NvsManager::getInstance().closeNVS();
}

void TeslaWiFiManager::storeConfiguration(){
    LOGI("Writing new configuration on the NVS");
    if(NvsManager::getInstance().openNVS(false,WIFI_SCHEMA_NAME)){

      configuredWiFi = 0;
      for (int i = 0; i < MAX_CONFIGURED_WIFI; i++) {
          if (strlen(wifiList[i].ssid) > 0) {
              configuredWiFi++;
          } else {
              break; // Esce al primo vuoto (no spazi vuoti)
          }
      }
      LOGV("storing %d wifi",configuredWiFi);
      
      NvsManager::getInstance().putInt("cfgwifi", configuredWiFi);

      // Salva solo le configurazioni valide (0 a configuredWiFi-1)
      for (int i = 0; i < configuredWiFi; i++)
      {
          char key[10];
          snprintf(key, sizeof(key), "wifi%d", i);
          
          JsonDocument newWifi;
          newWifi["ssid"] = wifiList[i].ssid;
          newWifi["psw"] = wifiList[i].password;
          String tmp;
          serializeJson(newWifi, tmp);
          NvsManager::getInstance().putString(key, tmp);
          LOGD("Saved WiFi %d: %s", i, wifiList[i].ssid);
      }

      // Pulisce tutto il resto con {}
      for(int i = configuredWiFi; i < MAX_CONFIGURED_WIFI; i++){
          char key[10];
          snprintf(key, sizeof(key), "wifi%d", i);
          NvsManager::getInstance().putString(key, "{}");
      }

    }

    NvsManager::getInstance().closeNVS();
}

#pragma endregion CONFIGURATION


#pragma region WiFiEvents
void TeslaWiFiManager::handleWiFiEvent(arduino_event_id_t event, arduino_event_info_t info) {

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY:
            LOGV("WiFi interface ready");
            break;

        case ARDUINO_EVENT_WIFI_AP_START:
            LOGV("WiFi access point started");
            apRunning = true;
            _dnsServer.start(53, "*",IPAddress(192,168,4,1));
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            _dnsServer.stop();
            apRunning = false;
            LOGV("WiFi access point stopped");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            scanDelayMillis = millis(); 
            copyWiFiList();
            break;

        case ARDUINO_EVENT_WIFI_STA_START:
            isWiFiSta = true;
            LOGV("WiFi client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            isWiFiSta = false;
            LOGV("WiFi clients stopped");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            oneMinMillis = 0;
            upTime = 0;
            scanStatus = scanStateEnum::SCAN_OFF;
            LOGV("Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            gotIP = false;
            staDisconnected = true;
            LOGV("%d",mainState);
            oneMinMillis = 0;
            upTime = 0;
            LOGV("Disconnected from WiFi access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            gotIP = true;
            LOGI("Obtained IP address:");
            LOGI("%s",WiFi.localIP().toString());
            break;

        default: break;

    }}

#pragma endregion


void TeslaWiFiManager::loop(){
    if(apRunning){ _dnsServer.processNextRequest(); }
    mainCycle();
}

void TeslaWiFiManager::mainCycle(){
      switch (mainState)
    {
    case DECISION:
      forceAPRequest = false;
      APcycle = AP_INIT;
      STAConCy = STA_INIT;
      scanDelayMillis = 0;
      lastFound = 0;
      if(configuredWiFi == 0){
        LOGI("Any configured wifi, going to AP");
        mainState = GOAP_RADIO_OFF;
        break;
      }
      LOGI("Some wifi was configured, going to STA");
      mainState = MAIN_STA_LOOP;
      break;

    case GOAP_RADIO_OFF:
      connectDirectly = false;
      APcycle = AP_INIT;
      STAConCy = STA_INIT;
      scanDelayMillis = 0;
      WiFi.disconnect(true,true);
      waitChange = millis();
      mainState = GOAP_WAIT_DELAY;
      break;

    case GOAP_WAIT_DELAY:
      if(millis() - waitChange >= 200){
        mainState = GOAP_RADIO_ON;
      }
      break;

    case GOAP_RADIO_ON:
      APcycle = AP_INIT;
      scanDelayMillis = 0;
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
      WiFi.softAP("TelsaBoard","123456789");
      waitChange = millis();
      mainState = GOAP_WAIT_AP_READY;
      break;

    case GOAP_WAIT_AP_READY:
      if(millis() - waitChange > 30000){
        LOGE("AP is not ready after 30sec...trying to restart");
        mainState = GOAP_RADIO_OFF;
      }
      if(!APisRunning()){
        break;
      }

      mainState = MAIN_AP_LOOP;
      LOGI("You can now connect to the WiFi Teslaboard the password is: 123456789");
      break;

    case MAIN_AP_LOOP:
      APloop();
      break;

    case GOSTA_RADIO_OFF:
      APcycle = AP_INIT;
      STAConCy = STA_INIT;
      scanDelayMillis = 0;
      WiFi.disconnect(true,true);
      waitChange = millis();
      mainState = GOSTA_WAIT_DELAY;
      break;
    case GOSTA_WAIT_DELAY:
      if(millis() - waitChange >= 200){
        mainState = GOSTA_RADIO_ON;
      }
      break;

    case GOSTA_RADIO_ON:
      LOGV("Enabling STA mode");
      WiFi.mode(WIFI_STA);
      mainState = GOSTA_WAIT_AP_READY;
      waitChange = millis();
      break;

    case GOSTA_WAIT_AP_READY:
      if(isWiFiSta){
        LOGV("STA mode enabled");
        mainState = MAIN_STA_LOOP;
      }
      if(millis() - waitChange > 30000){
        LOGE("Unable to turn on STA, trying to shuwtdow the radio");
        mainState = GOSTA_RADIO_OFF;
      }
      break;

    case MAIN_STA_LOOP:
      STAloop();
      break;
    
    default:
      LOGE("UNDEFINED STEP");
      break;
    }

}

#pragma region  AP Cycle

void TeslaWiFiManager::APloop(){
  /*
  I'm in AP STA mode....
  Async scan,
  when is done check if configured wifi was found.
  or wait for a new wifi
  */
  scanManager();

  //if AP was requested wait 5 minutes before trying to connect
  if(forceAPRequest && (millis() - apRequestMillis >= 300000)){
      LOGI("AP request auto disabled after 5 mniutes");
      forceAPRequest = false;
  }
  

  switch (APcycle)
  {
  case AP_INIT: //run the wifi scan
    LOGV("Starting the scan operation from AP");
    scanStatus = scanStateEnum::SCAN_WAIT_START;
    APcycle = AP_WAIT_OPERATION;
    lastFound = 0;
    break;

  case AP_WAIT_OPERATION: //waiting to find a configured wifi or a new wifi
    if(scanStatus == scanStateEnum::SCAN_DONE){
      if(findMatchingWiFi()){
        if(forceAPRequest){
          LOGV("One WiFi was found, but AP was requested by the user");
        }else{
          LOGV("One WiFi was found, trying to connect");
          connectDirectly = true;
          mainState = GOSTA_RADIO_OFF;
        }
      }
      scanStatus = scanStateEnum::SCAN_WAIT_START;
    }
    if(scanStatus == scanStateEnum::SCAN_TIMEOUT){
        LOGV("Unable to scan wifi, reboot wifi driver");
        mainState = GOAP_RADIO_OFF;
    }

    if(incomingWiFi){
      APcycle = AP_CONNECT_TO_WIFI;
    }
    break;

  case AP_CONNECT_TO_WIFI:
    LOGV("Trying to connect");
    incomingWiFi = false; // consuma la richiesta
    gotIP = false;
    staDisconnected = false;
    connectToWifi(wifiToConnect["ssid"].as<const char*>(),wifiToConnect["psw"].as<const char*>());
    APcycle = AP_WAIT_CONNECTION;
    break;

  case AP_WAIT_CONNECTION:
    if(WiFi.status() == WL_CONNECTED){
      storeNewWiFi(wifiToConnect["ssid"].as<String>(),wifiToConnect["psw"].as<String>());
      if(configuredWiFi > 0){
        lastFound = configuredWiFi -1;
        connectDirectly = true;
      }
      LOGV("Connection was ok, going in STA mode");
      mainState = GOSTA_RADIO_OFF;
    } else if((millis()- connectionTOUTMillis >= 10000 || staDisconnected)){
      LOGE("Unable to connect");
      APcycle = AP_INIT;
    }
    break;
  
  default:
    LOGE("UNDEFINED STEP");
    break;
  }
}

#pragma endregion

#pragma region STA Cycle

void TeslaWiFiManager::STAloop(){

    STAConnectionCycle();


  if(!digitalRead(0)){
    if(!gpio0pressed){
      LOGV("GPIO 0 pressed");
      gpio0pressed= true;
      apRequestMillis = millis();
    } else {
      if(millis()- apRequestMillis >= 1000){
        LOGW("Captive Portal requested, going to AP");
        forceAPRequest = true;
        apRequestMillis = millis();
        mainState = GOAP_RADIO_OFF;
      }
    }
  } else {
    gpio0pressed = false;
  }

}



void TeslaWiFiManager::STAConnectionCycle(){


  scanManager();
  switch (STAConCy)
  {
  case STA_INIT:
   staDisconnected = false;
    if(connectDirectly){
      LOGV("A Wifi was found from AP, connect directly");
      STAConCy = STA_CONNECT_TO_WIFI;
      connectDirectly = false;
    } else {
      LOGV("Scanning for configured wifi");
      scanStatus = SCAN_WAIT_START;
      STAConCy = STA_WAIT_SCAN;
    }

    break;

  case STA_WAIT_SCAN:
    if(scanStatus == scanStateEnum::SCAN_DONE){
      STAConCy = STA_BEFORE_CONNECT;
    } else if(scanStatus == scanStateEnum::SCAN_TIMEOUT){
      LOGV("Scan goes in timeout, trying to restart the wifi");
      mainState = GOSTA_RADIO_OFF; // if scan don't work we need to reboot the wifi
    }
    break;

  case STA_BEFORE_CONNECT:
    scanStatus = scanStateEnum::SCAN_OFF;//stop the scan operation
    LOGV("Stop the scan operations");
    STAConCy = STA_LOOK_FOR_WIFI;
    lastFound = 0; //reset the comparing pointer
    break;

  case STA_LOOK_FOR_WIFI:

    if(findMatchingWiFi()){
      STAConCy = STA_CONNECT_TO_WIFI;
      LOGV("Configured WiFi was found");
    } else {
      STAConCy = STA_GO_TO_AP;
      LOGV("No Configured WiFi was found, going to AP");
    }
    break;

  case STA_GO_TO_AP:
    LOGV("Any wifi found, going to AP");
    STAConCy = STA_INIT;
    mainState = GOAP_RADIO_OFF;
    break;

  case STA_CONNECT_TO_WIFI:
    connectDirectly = false;
    LOGV("%d",lastFound);
    LOGV("Try to connect to the wifi %s",wifiList[lastFound].ssid);
    connectToWifi(wifiList[lastFound].ssid,wifiList[lastFound].password);
    STAConCy = STA_WAIT_CONNECTION;
    break;

  case STA_WAIT_CONNECTION:
    if (gotIP && WiFi.status() == WL_CONNECTED &&
        WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
        LOGI("WiFi connected, IP: %s", WiFi.localIP().toString().c_str());
        STAConCy = STA_CONNECTED;
    } 
    else if ((millis() - connectionTOUTMillis > 10000) || WiFi.status() == WL_CONNECT_FAILED || staDisconnected) {
        LOGW("Unable to connect");
        gotIP = false;
        staDisconnected = false;
        lastFound++;
        STAConCy = STA_DISCONNECT;
    }
    break;

  case STA_CONNECTED:
    if(WiFi.status() == WL_CONNECTED && gotIP && WiFi.localIP() != IPAddress(0, 0, 0, 0)){
        if(oneMinMillis == 0){ 
          oneMinMillis = millis(); 
        } else if(millis()-oneMinMillis >= 60000){
            oneMinMillis = millis();
            upTime++;
        }
    } else {
        oneMinMillis = 0;
        upTime = 0;
        LOGV("WiFi Disconnected");
        STAConCy = STA_DISCONNECT;
    }
    break;


  case STA_DISCONNECT:
      LOGV("Disconnetting WiFi");
      WiFi.disconnect(true);
      STAConCy = STA_TURN_ON_WIFI;
      waitChange = millis();
      break;

  case STA_TURN_ON_WIFI:
      if(millis() - waitChange < 200){
          break;
      }
      LOGV("WiFi in STA mode");
      WiFi.setHostname(hostName.c_str());
      WiFi.mode(WIFI_STA);
      STAConCy = STA_INIT;
      break;

  case STA_WAIT_RADIO_ON:
      if(!isWiFiSta){
          break;
      }
      STAConCy = STA_LOOK_FOR_WIFI;
      break;
  default:
    LOGE("UNDEFINED STEP");
    break;
  }
}

#pragma endregion

#pragma region Scan Functions

void TeslaWiFiManager::scanManager(){

  switch (scanStatus)
  {
  case SCAN_OFF:
    scanTimeOutMillis = 0;
    scanDelayMillis = 0;
    return;

  case SCAN_SCANNING:
    if(scanTimeOutMillis == 0){ scanTimeOutMillis = millis(); }
    if(millis() - scanTimeOutMillis > 30000){
      LOGE("WiFiScan Time Out");
      scanStatus = scanStateEnum::SCAN_TIMEOUT;
    }
    break;

  case SCAN_WAIT_START:
  case SCAN_DONE:
    if(scanDelayMillis == 0){ startWiFiscan(); } //start asap
    
    else if(millis() - scanDelayMillis >= 30000 ){
      startWiFiscan();
    }
    break;

  case SCAN_TIMEOUT:
    break;
  }


}

bool TeslaWiFiManager::findMatchingWiFi() {

  for (int i = lastFound; i < configuredWiFi; i++)
  {
    for (int x = 0; x < wifiScanList.size(); x++) {
      JsonObject wifi = wifiScanList[x].as<JsonObject>();
      if (strcmp(wifiList[i].ssid, wifi["ssid"].as<const char*>()) == 0) {
        LOGV("Match: %s", wifiList[i].ssid);
        lastFound = i;
        return true;
      }
    }
  }
  return false;
}

void TeslaWiFiManager::startWiFiscan(){
      scanTimeOutMillis = millis();
      WiFi.scanNetworks(true);
      scanStatus = scanStateEnum::SCAN_SCANNING;
}

void TeslaWiFiManager::copyWiFiList(){
  int found = WiFi.scanComplete();
  wifiScanList.clear();
  JsonArray wifiListArray = wifiScanList.to<JsonArray>();

  for (int i = 0; i < found; i++) {
    JsonObject wifi = wifiListArray.add<JsonObject>();
    wifi["ssid"] = WiFi.SSID(i);
    wifi["enc"]  = WiFi.encryptionType(i);
    wifi["rssi"] = WiFi.RSSI(i);
  }

  for (int i = 0; i < found - 1; i++) {
    for (int j = 0; j < found - 1 - i; j++) {
      JsonObject a = wifiListArray[j];
      JsonObject b = wifiListArray[j + 1];

      int rssiA = a["rssi"];
      int rssiB = b["rssi"];

      if (rssiA < rssiB) {
        String ssidA = a["ssid"].as<String>();
        int   encA   = a["enc"];

        String ssidB = b["ssid"].as<String>();
        int   encB   = b["enc"];

        a["ssid"] = ssidB;
        a["enc"]  = encB;
        a["rssi"] = rssiB;

        b["ssid"] = ssidA;
        b["enc"]  = encA;
        b["rssi"] = rssiA;
      }
    }
  }

  WiFi.scanDelete();
  scanStatus = scanStateEnum::SCAN_DONE;
}

#pragma endregion

void TeslaWiFiManager::connectToWifi(const char* ssid, const char* password){
  gotIP = false;
  LOGV("Connecting to: %s",ssid);
  WiFi.setHostname(hostName.c_str());
  WiFi.begin(ssid,password);
  connectionTOUTMillis = millis();
  staDisconnected = false;
}

void TeslaWiFiManager::storeNewWiFi(String ssid,String password){
  if(configuredWiFi == MAX_CONFIGURED_WIFI){ return; }
  int i = configuredWiFi;
  strlcpy(wifiList[i].ssid, ssid.c_str(), 33);
  strlcpy(wifiList[i].password, password.c_str(), 64);
  configuredWiFi++;
  storeConfiguration();
}

void TeslaWiFiManager::removeWiFiById(int id) {
  if (id < 0 || id >= MAX_CONFIGURED_WIFI) return;

  for (int i = id; i < MAX_CONFIGURED_WIFI - 1; i++) {
    wifiList[i] = wifiList[i + 1];
  }
  strcpy(wifiList[MAX_CONFIGURED_WIFI - 1].ssid, "");
  strcpy(wifiList[MAX_CONFIGURED_WIFI - 1].password, "");

  if(configuredWiFi > 0){
    configuredWiFi--;
  }
  storeConfiguration();
  
}

#pragma region web
void TeslaWiFiManager::web(){
    
    //wifi list
    _server->on("/wifi-api/wifi-list", HTTP_GET, [this](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["mode"] = WiFi.getMode();
        doc["connected"] = WiFi.status() == WL_CONNECTED;
        doc["wifi"].set(wifiScanList);
        JsonArray wifiStored = doc["stored"].to<JsonArray>();
        for (int i = 0; i < configuredWiFi; i++) {
          JsonObject obj = wifiStored.add<JsonObject>();
          obj["ssid"] = wifiList[i].ssid;
          obj["psw"]  = wifiList[i].password;
        }

        response->setLength();
        request->send(response);
    });


    //new wifi to be stored
    AsyncCallbackJsonWebHandler* incomingWiFi = new AsyncCallbackJsonWebHandler("/wifi-api/new-wifi");
    incomingWiFi->setMethod(HTTP_POST | HTTP_PUT);
    incomingWiFi->onRequest([this](AsyncWebServerRequest* request, JsonVariant& root) {

        if(root["ssid"] == ""){
            request->send(400, "application/json", "{\"error\":\"no ssid\"");
            return;
        }
        
        if(root["ssid"].as<String>().length() > 32){
            request->send(400, "application/json", "{\"error\":\"SSID too long\"}");
            return;
        }
        int pswLenght = root["psw"].as<String>().length();
        if(pswLenght != 0 &&  pswLenght < 8){
            request->send(400, "application/json", "{\"error\":\"Password too short\"}");
            return;
        }
        if(root["psw"].as<String>().length() > 63){
            request->send(400, "application/json", "{\"error\":\"Password too long\"}");
            return;
        }
        wifiToConnect.clear();
        wifiToConnect["ssid"] = root["ssid"].as<String>();
        wifiToConnect["psw"] = root["psw"].as<String>();
        this->incomingWiFi = true;


        request->send(200, "text/plain", "{\"executed\":true}");
    });
    _server->addHandler(incomingWiFi);


    //delete wifi
    _server->on("/wifi-api/delete-wifi", HTTP_POST, [this](AsyncWebServerRequest *request) {
        int id = -1;
        
        if (request->hasParam("id", true)) {  // true = dal body
          id = request->getParam("id", true)->value().toInt();
          if(id>=0 && id<MAX_CONFIGURED_WIFI){
            removeWiFiById(id);
          }          
          request->send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
          request->send(400, "application/json", "{\"error\":\"Missing ID\"}");
        }
      });


    _server->serveStatic("/wifi-mgr", LittleFS, "/www/").setDefaultFile("wifi.html");

}

#pragma endregion

