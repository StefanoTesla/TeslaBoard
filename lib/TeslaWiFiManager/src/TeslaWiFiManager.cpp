#include "TeslaWiFiManager.h"
#include "esp_log.h"
#define LOG_TAG "WiFiMgr"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)


TeslaWiFiManager::TeslaWiFiManager(AsyncWebServer *server) : _server(server) {}


#pragma region nvsHandler

bool TeslaWiFiManager::openNVS(bool readOnly) {

  switch (nvsStatus) {

    // nvs is closed, i need to ope according by the readOnly
  case CLOSED:
    LOGV("NVS seems to be closed");
    if (nvs.begin(WIFI_SCHEMA_NAME, readOnly)) {
      if (readOnly) {
        LOGV("NVS opened in readonly");
        nvsStatus = OPEN_READOLNY;
      } else {
        LOGV("NVS opened with write rights");
        nvsStatus = OPEN_WRITE;
      }
      return true;
    } else {
      LOGE("Error opening the NVS");
      nvsStatus = CLOSED;
      return false;
    }
    break;

  case OPEN_READOLNY:
    LOGV("NVS seems open in read only");
    if (!readOnly) {
      closeNVS();
      if (nvs.begin(WIFI_SCHEMA_NAME, false)) {
        nvsStatus = OPEN_WRITE;
        LOGV("NVS opened with write rights");
        return true;
      } else {
        LOGV("Error during opening NVS with write rights");
        return false;
      }
    } else {
      LOGV("NVS already open in read only");
      return true;
    }
    break;

  case OPEN_WRITE:
    LOGV("NVS seems open with write rights");
    if (readOnly) {
      closeNVS();
      nvsStatus = CLOSED;
      if (nvs.begin(WIFI_SCHEMA_NAME, true)) {
        nvsStatus = OPEN_READOLNY;
        LOGV("NVS opened in read only");
        return true;
      } else {
        LOGV("Error during opening NVS in read only");
        return false;
      }
    } else {
      LOGV("NVS already open with write rights");
      return true;
    }
    break;

  default:
    LOGE("Unknown NVS status: %d", nvsStatus);
    return false;
    break;
  }

  LOGE("Arrived at the buttom of the function, don't know what happed..");
  return false;
}

void TeslaWiFiManager::closeNVS() {
  if (nvsStatus != CLOSED) {
    nvs.end();
    nvsStatus = CLOSED;
    LOGV("NVS closed");
  } else {
    LOGV("NVS already closed");
  }
}

#pragma endregion

#pragma region CONFIGURATION
/* initialize the board */
void TeslaWiFiManager::begin() {
    LOGI("Loading configuration");
    JsonDocument doc;

    if (!openNVS(true)) {
        LOGE(
            "Error loading wifi nvs partition in read only, trying to format it");
        if (!initNVS()) {
        LOGE("NVS INITIALIZATION FAILED");
        return;
        }
    }

    LOGV("Checking the schema version");
    // if I'm here NVS is surelly working, no more check...
    openNVS(true);
    int schemaVersion = nvs.getInt("schema", 0);
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

    if(nvsStatus != OPEN_READOLNY){ openNVS(true); }
    
    configuredWiFi = nvs.getInt("cfgwifi", 0);

    for (int i = 0; i < configuredWiFi ; i++){
      tmpCfg.clear();
      char key[10];
      snprintf(key, sizeof(key), "wifi%d", i);
      String wifiX= nvs.getString(key,"{}");
      deserializeJson(tmpCfg,wifiX);
      if(tmpCfg.size() != 0){
        const char* ssid = tmpCfg["ssid"] | "";  // default vuoto se non esiste
        const char* password = tmpCfg["password"] | "";
        
        strlcpy(wifiList[i].ssid, ssid, sizeof(wifiList[i].ssid));
        strlcpy(wifiList[i].password, password, sizeof(wifiList[i].password));
        wifiList[i].preferred = tmpCfg["preferred"] | false;
        Serial.println(i);
        Serial.println(wifiList[i].ssid);
      }
    }

    closeNVS();
    web();

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info){
      this->handleWiFiEvent(event, info);
    });


    if(configuredWiFi == 0){
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP("TeslaBoard", "123456789");
      WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
    } else {
      WiFi.mode(WIFI_STA);
    }


    WiFi.setAutoReconnect(false);
}

bool TeslaWiFiManager::initNVS() {

  LOGW("Board nvs area will be formatted");
  if (!openNVS(false)) {
    LOGE("Unable to open the namespace with write rights, initialization failed");
    return false;
  }
  LOGI("namespace open or created, writing default parameters");

  nvs.putInt("schema", 0);
  nvs.putInt("cfgwifi", 0);
  
  closeNVS();
  return true;
}

void TeslaWiFiManager::updateNVS1() {
  // this is the first schema, don't check if something already exist.
  openNVS(false);

  nvs.putInt("schema", 1);

  closeNVS();
}

void TeslaWiFiManager::getConfiguration(JsonObject dest){
}

void TeslaWiFiManager::storeConfiguration(){
    LOGI("Writing new configuration on the NVS");
    openNVS(false);

    nvs.putInt("cfgwifi", configuredWiFi);

    for (int i = 0; i < configuredWiFi; i++)
    {
      tmpCfg.clear();
      char key[10];
      snprintf(key, sizeof(key), "wifi%d", i);
      
      JsonDocument newWifi;

      newWifi["ssid"] = wifiList[i].ssid;
      newWifi["psw"] = wifiList[i].password;
      String tmp;
      serializeJson(newWifi,tmp);
      serializeJson(newWifi,Serial);
      nvs.putString(key,tmp);
    }
    
    closeNVS();

}

#pragma endregion CONFIGURATION

void TeslaWiFiManager::handleWiFiEvent(arduino_event_id_t event, arduino_event_info_t info) {
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY:
            oneMinMillis = 0;
            upTime = 0;
            Serial.println("WiFi interface ready");
            break;

        case ARDUINO_EVENT_WIFI_AP_START:
            Serial.println("WiFi access point started");
            apRunning = true;
            _dnsServer.start(53, "*",IPAddress(192,168,4,1));
            okToScan = true;
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            _dnsServer.stop();
            apRunning = false;
            Serial.println("WiFi access point stopped");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            scanInProgress = false;
            scanDelayMillis = millis(); 
            copyWiFiList();
            break;

        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("WiFi client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            Serial.println("WiFi clients stopped");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            oneMinMillis = 0;
            upTime = 0;
            if(toBeStored){
              toBeStored = false;
              storeNewWiFi();
            }
            okToScan = false;
            Serial.println("Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            oneMinMillis = 0;
            upTime = 0;
            Serial.println("Disconnected from WiFi access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("Obtained IP address: ");
            Serial.println(WiFi.localIP());
            break;

/*UNUSED EVENTS
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Serial.println("Lost IP address and IP address is reset to 0");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            Serial.println("Client connected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            Serial.println("Client disconnected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            Serial.println("Assigned IP address to client");
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            Serial.println("Received probe request");
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            Serial.println("AP IPv6 is preferred");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Serial.println("STA IPv6 is preferred");
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            Serial.println("Authentication mode of access point has changed");
            break;
            */
        default: break;

    }}

void TeslaWiFiManager::loop(){
    if(apRunning){ _dnsServer.processNextRequest(); }
    
    if(WiFi.status() == WL_CONNECTED){
      if(oneMinMillis == 0) { oneMinMillis = millis();}
      if(millis() - oneMinMillis >= 60000){
          oneMinMillis = millis();
          upTime++;
      }
    } else {

    }


    scanManager();
}

void TeslaWiFiManager::scanManager(){
  if(okToScan == false){
    scanTimeOutMillis = 0;
    scanDelayMillis = 0;
    return;
  }

  if(scanInProgress){
      if(scanTimeOutMillis == 0){ scanTimeOutMillis = millis(); }
      if(millis() - scanTimeOutMillis > 30000){
        LOGE("WiFiScan Time Out");
        startWiFiscan();
      }
  } else {
    if(scanDelayMillis == 0){
      startWiFiscan();
    } else if(millis() - scanDelayMillis >= 30000 ){
      startWiFiscan();
    }
  }

}

void TeslaWiFiManager::copyWiFiList(){
  int found = WiFi.scanComplete();
  wifiScanList.clear();
  JsonArray wifiListArray = wifiScanList.to<JsonArray>();

  for (int i = 0; i < found; i++)
  {
    JsonObject wifi = wifiListArray.add<JsonObject>();
    wifi["ssid"] = WiFi.SSID(i);
    wifi["enc"] = WiFi.encryptionType(i);
    wifi["rssi"] = WiFi.RSSI(i);
  }

  WiFi.scanDelete(); 
}

void TeslaWiFiManager::startWiFiscan(){
      Serial.println("Start scan");
      scanTimeOutMillis = millis();
      WiFi.scanNetworks(true);
      scanInProgress = true;
}

void TeslaWiFiManager::connectToWifi(){
  WiFi.begin(
    wifiToConnect["ssid"].as<String>().c_str(),
    wifiToConnect["psw"].as<String>().c_str());
}

void TeslaWiFiManager::storeNewWiFi(){
  int i = configuredWiFi;
  strlcpy(wifiList[i].ssid, wifiToConnect["ssid"].as<const char*>(), 33);
  strlcpy(wifiList[i].password, wifiToConnect["psw"].as<const char*>(), 64);
  configuredWiFi++;
  storeConfiguration();
}


void TeslaWiFiManager::deleteWiFi(int id){

  if(id < 0 || id >= configuredWiFi) {
        LOGE("Unable to delete wifi number %d outside the limits",id);
        return;  // ID non valido
  }

  for(int i = id; i < configuredWiFi - 1; i++){
      wifiList[i] = wifiList[i+1];
  }
      
  configuredWiFi--;
}


void TeslaWiFiManager::web(){
    
    //wifi list
    _server->on("/wifi-api/wifi-list", HTTP_GET, [this](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["mode"] = WiFi.getMode();
        doc["connected"] = WiFi.status() == WL_CONNECTED;
        doc["wifi"].set(wifiScanList);

        response->setLength();
        request->send(response);
    });


    //new wifi to be stored
    AsyncCallbackJsonWebHandler* incomingWiFi = new AsyncCallbackJsonWebHandler("/wifi-api/new-wifi");
    incomingWiFi->setMethod(HTTP_POST | HTTP_PUT);
    incomingWiFi->onRequest([this](AsyncWebServerRequest* request, JsonVariant& root) {

        Serial.print("[WiFiMgr] New wifi incoming: ");
        Serial.println(root["ssid"].as<String>());
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
        toBeStored = true;
        connectToWifi();

        request->send(200, "text/plain", "{\"executed\":true}");
    });
    _server->addHandler(incomingWiFi);


    //delete wifi
    AsyncCallbackJsonWebHandler* deleteWiFi = new AsyncCallbackJsonWebHandler("/wifi-api/delete-wifi");
    deleteWiFi->setMethod(HTTP_POST | HTTP_PUT);
    deleteWiFi->onRequest([&](AsyncWebServerRequest* request, JsonVariant& root) {

        request->send(200, "text/plain", "{\"executed\":true}");
        
    });
    _server->addHandler(deleteWiFi);


    _server->serveStatic("/wifi-mgr", LittleFS, "/www/").setDefaultFile("wifi.html");

}