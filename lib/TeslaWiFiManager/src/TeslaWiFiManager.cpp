#include "TeslaWiFiManager.h"

TeslaWiFiManager::TeslaWiFiManager(AsyncWebServer *server) : _server(server) {}

//startup cycle
void TeslaWiFiManager::init(){

    pinMode(0,INPUT);
    if(!_routeInit){
        initNVS();
        serverRouting(); 
        WiFi.mode(WIFI_STA); //turn on wifi
        _routeInit = true;
    }
    unsigned int ackmillis=0;
    while (1)
    {
        switch (_cycle)
            {
            case INIT:
                //check if a wifi is valid
                
                if(storedWifi()>0){
                    _cycle = CONNECT_TO_STORED_WIFI;
                } else {
                    _cycle = BEFORE_START_AP;
                    Serial.println("[WiFiMgr] No stored Wifi founds, going to AP");
                }
                break;

            case CONNECT_TO_STORED_WIFI: {
                    bool connected = false;
                    for (size_t i = 0; i < MAX_WIFI_NETWORKS; i++)
                    {
                        if(_savedNetworks[i].isValid){
                            connect( _savedNetworks[i]);
                            _lms = millis();
                            if(connectionWait()){
                                connected = true;
                                break;
                            }
                        }
                    }
                    
                    if(connected){
                        Serial.println("[WiFiMgr] Connected, enjoy!");
                        _cycle = END;
                    } else {
                        Serial.println("[WiFiMgr] Unable to connect to any wifi, going in AP");
                        _cycle = BEFORE_START_AP;
                    }
                }
                break;

            case BEFORE_START_AP:
                //start the async wifi
                startWiFiScan();
                _cycle = AP_START;
                break;

                //goes to AP
            case AP_START:
                waitWiFiScanCompleted();
                APstart();
                _cycle = WAIT_AP_RUNNING;
                break;

            case WAIT_AP_RUNNING:
                while(WiFi.softAPIP() != IPAddress(192,168,4,1)){
                    ;
                }
                _cycle = AP_LOOP;
                serverBegin();
                Serial.println("[WiFiMgr] Waiting for WiFi data");
                _lms = millis();
                break;
            case AP_LOOP:
                APLoop();
                break;

            case AP_STOP:
                APstop();
                _cycle = CONNECT_TO_WIFI;
                break;
            case CONNECT_TO_WIFI:
                connect(_toConnectNetwork);
                _cycle = WAIT_FOR_CONNECTION;
                _lms = millis();
                break;

            case WAIT_FOR_CONNECTION:
                if(connectionWait()){
                    _cycle = STORE_INCOMING_WIFI;
                }
                else {
                    _newIncomingWiFi = false;
                    _cycle = BEFORE_START_AP;
                }
                break;

            case STORE_INCOMING_WIFI:
                if(_newIncomingWiFi){
                    _newIncomingWiFi = false;
                    storeWiFiConnection();
                }
                _cycle = END;
                break;

            case END:
                Serial.print("[WiFiMgr] TeslaBoard IP address is: ");
                Serial.println(WiFi.localIP());
                return;
                break;             
            default:
                break;
        }

        if(_dnsServerActive){
            _dnsServer.processNextRequest();
        }
    }
    
}

//loop cycle
void TeslaWiFiManager::loop(){

    switch (_loopCycle)
    {
    case L_LOOP:
        //check if wifi is disconnected
        if(WiFi.status() != WL_CONNECTED){
                _loopCycle = L_CONNECTION_RETRY;
                _reconnectTimeOut = millis();
                break;
        }
        

        break;

    case L_CONNECTION_RETRY:
        if((millis() - _reconnectTimeOut) < 11000){
            Serial.println("[WiFiMgr L] Trying to reconnect...");
            WiFi.reconnect();
            _loopCycle = L_CONNECTION_CHECK;
            _lms=millis();
        } else {
            //to many times tring to reconnect...
            _loopCycle = L_DISCONNECT_WIFI;
        }

        break;

    case L_CONNECTION_CHECK:
        if(WiFi.status() == WL_CONNECTED){
            printIP();
            _loopCycle = L_LOOP;
            break;
        } else {
            if(millis() - _lms > 5000){
                _loopCycle = L_CONNECTION_RETRY;
            }
        }
        break;


    case L_DISCONNECT_WIFI:
        WiFi.disconnect(true); 
        _loopCycle = L_SHUTDOWN_WIFI;
        _lms = millis();
        break;

    case L_SHUTDOWN_WIFI:
        if(WiFi.getMode() == WIFI_OFF){
            Serial.println("[WiFiMgr L] set the WiFi on station mode");
            WiFi.mode(WIFI_STA); //workaround to make scan working
            _loopCycle = L_BACK_TO_STA_MODE;
        }
        break;

    case L_BACK_TO_STA_MODE:
        if(WiFi.getMode() == WIFI_STA){
            Serial.println("[WiFiMgr L] WiFi is on station mode");
            _lms = millis();
            _loopCycle = L_START_SCAN;
        }
        break;

    case L_START_SCAN:
        startWiFiScan();
        _loopCycle = L_WAIT_SCAN;
        break;
            
    case L_WAIT_SCAN:
        if(asyncWaitWiFiScan()){
            _loopCycle = L_CHECK_CONFIGURED_WIFI;
        }
        break;

    case L_CHECK_CONFIGURED_WIFI:
        if(checkScannedNetworks()){
            Serial.println("[WiFiMgr L] Configured Wifi was found, try to connect");
            _loopCycle = L_START_CONNECTION_TO_CONFIGUERD_WIFI;
            _SSIDWiFiPointer = 0;
            _StoredWiFiPointer = 0;
        } else {
            Serial.println("[WiFiMgr L] Any configured Wifi was found, waiting for a new scan..");
            _loopCycle = L_WAIT_FOR_A_NEW_RESCAN;
            _lms = millis();
        }
        break;

    case L_WAIT_FOR_A_NEW_RESCAN:
            if(millis() - _lms > 30000){
                //WiFi.disconnect(true); //workaround to make scan working
                _loopCycle = L_START_SCAN;
            }
        break;

    case L_START_CONNECTION_TO_CONFIGUERD_WIFI:
        
        if(_skipThisNetwork){
            _skipThisNetwork = false;
            _StoredWiFiPointer++;
        }

        if(_StoredWiFiPointer >= storedWifi()){
            //tested all stored connections
            //but something get wrong...
            //try to perfomr a new rescan
            Serial.println("[WiFiMgr L] Tried to connect on each stored WiFi without success, perform a new scan..");
            _loopCycle = L_DISCONNECT_WIFI;
            break;
        }

        if(_savedNetworks[_StoredWiFiPointer].isValid && (WiFi.SSID(_SSIDWiFiPointer) != _savedNetworks[_StoredWiFiPointer].ssid)){

            _SSIDWiFiPointer++;
            if(_SSIDWiFiPointer >= _wifiNetworkFound){
                _SSIDWiFiPointer = 0;
                _StoredWiFiPointer++;
            }
        } else {

            //try to connect to the stored and founded SSID
            connect(_savedNetworks[_StoredWiFiPointer]);
            _lms = millis();
            _loopCycle = L_WAIT_CONNECTION_TO_STORED_WIFI;
        }
        break;

    case L_WAIT_CONNECTION_TO_STORED_WIFI:
        if(WiFi.status() == WL_CONNECTED){
            _SSIDWiFiPointer = 0;
            _StoredWiFiPointer = 0;
            printIP();
            _loopCycle = L_LOOP;
            break;
        }

        if(millis()- _lms > 5000){
            Serial.println("[WiFiMgr L] Unable to connect!");
            _skipThisNetwork = true;
            _loopCycle = L_START_CONNECTION_TO_CONFIGUERD_WIFI;
        }
        break;


    default:
        break;
    }
}

void TeslaWiFiManager::startWiFiScan(){
    if(_connectionTentative){
        WiFi.disconnect(true);
    }
    Serial.println("[WiFiMgr] Starting Scan...");
    int scan = WiFi.scanNetworks(true);

    Serial.println(scan);
    Serial.println("[WiFiMgr] Scan in progress...");
    _scanInProgress = true;
}

void TeslaWiFiManager::waitWiFiScanCompleted(){
    
    while(WiFi.scanComplete() < 0  ){
       ;
    }
    Serial.println("[WiFiMgr] Scan Completed");
    _wifiNetworkFound = WiFi.scanComplete();
    _scanInProgress = false;
}

bool TeslaWiFiManager::asyncWaitWiFiScan(){
    _wifiNetworkFound = WiFi.scanComplete();
    if(_wifiNetworkFound < 0  ){
        return false;
    }
    Serial.print("[WiFiMgr] Scan during AP Completed: ");
    Serial.print(_wifiNetworkFound);
    Serial.println(" network found.");
    _scanInProgress = false;
    return true;
}

void TeslaWiFiManager::APstart(){
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("TeslaBoard", "123456789");
    WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
    _dnsServer.start(53, "*",IPAddress(192,168,4,1));
    _dnsServerActive = true;
}

void TeslaWiFiManager::APstop(){
    _dnsServer.stop();
    _dnsServerActive = false;
    serverStop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("[WiFiMgr] AP closed");    
}

void TeslaWiFiManager::APLoop(){
    //new connection request

    if(_newIncomingWiFi){
        _cycle = AP_STOP;
        return;
    }

    // background scanning every 30seconds
    // if a configured WiFi is found try to connect to..
    if(millis() - _lms > 30000){

        if(!_scanInProgress){
            startWiFiScan();
        } else {
            if(asyncWaitWiFiScan()){
                _lms = millis();
                if(checkScannedNetworks()){
                    _cycle = AP_STOP;
                    Serial.println("[WiFiMgr] Configured Wifi found, try to connect...");
                } else {
                    Serial.println("[WiFiMgr] Any configured Wifi found...");
                }
            }
        }
                    
    }
}

void TeslaWiFiManager::setStaticIP(IPAddress ip, IPAddress gw,IPAddress sn){

}

void TeslaWiFiManager::serverRouting(){
    

    //wifi list
    _server->on("/wifi-mgr/api/wifi-list", HTTP_GET, [&](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["mode"] = WiFi.getMode();
        doc["connected"] = WiFi.status() == WL_CONNECTED;
        if(_scanInProgress){
            doc["scanning"] = true;
        } else {
            JsonArray wifiList = doc["wifi"].to<JsonArray>();
            for (size_t i = 0; i < _wifiNetworkFound; i++)
            {
                wifiList[i]["ssid"] = WiFi.SSID(i);
                wifiList[i]["rssi"] = WiFi.RSSI(i);
                wifiList[i]["enc"] = "psw";
                if(WiFi.encryptionType(i) == 0){
                    wifiList[i]["enc"] = "open";
                }
            }
        }

        JsonArray wifiConfigured = doc["stored"].to<JsonArray>();
        for (size_t i = 0; i < MAX_WIFI_NETWORKS; i++)
        {
            if(_savedNetworks[i].isValid){
                JsonObject stored = wifiConfigured.add<JsonObject>();
                stored["ssid"] = _savedNetworks[i].ssid; 
                stored["default"] = _savedNetworks[i].pref;
            }
        }

        response->setLength();
        request->send(response);
    });

    //wifi list
    _server->on("/wifi-api/wifi-list", HTTP_GET, [&](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["mode"] = WiFi.getMode();
        doc["connected"] = WiFi.status() == WL_CONNECTED;
        if(_scanInProgress){
            doc["scanning"] = true;
        } else {
            JsonArray wifiList = doc["wifi"].to<JsonArray>();
            for (size_t i = 0; i < _wifiNetworkFound; i++)
            {
                wifiList[i]["ssid"] = WiFi.SSID(i);
                wifiList[i]["rssi"] = WiFi.RSSI(i);
                wifiList[i]["enc"] = "psw";
                if(WiFi.encryptionType(i) == 0){
                    wifiList[i]["enc"] = "open";
                }
            }
        }

        JsonArray wifiConfigured = doc["stored"].to<JsonArray>();
        for (size_t i = 0; i < MAX_WIFI_NETWORKS; i++)
        {
            if(_savedNetworks[i].isValid){
                JsonObject stored = wifiConfigured.add<JsonObject>();
                stored["ssid"] = _savedNetworks[i].ssid; 
                stored["default"] = _savedNetworks[i].pref;
            }
        }

        response->setLength();
        request->send(response);
    });


    //new wifi to be stored
    AsyncCallbackJsonWebHandler* incomingWiFi = new AsyncCallbackJsonWebHandler("/wifi-api/new-wifi");
    incomingWiFi->setMethod(HTTP_POST | HTTP_PUT);
    incomingWiFi->onRequest([&](AsyncWebServerRequest* request, JsonVariant& root) {

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

        if(storedWifi() >= MAX_WIFI_NETWORKS){
            request->send(400, "text/plain", "{\"error\":\"no space, delete some wifi\"");
            return;
        }

        _toConnectNetwork.ssid = root["ssid"].as<String>();
        _toConnectNetwork.password = root["psw"].as<String>();
        _toConnectNetwork.pref = root["default"].as<bool>();
        _newIncomingWiFi = true;
        request->send(200, "text/plain", "{\"executed\":true}");
    });
    _server->addHandler(incomingWiFi);


    //delete wifi
    AsyncCallbackJsonWebHandler* deleteWiFi = new AsyncCallbackJsonWebHandler("/wifi-api/delete-wifi");
    deleteWiFi->setMethod(HTTP_POST | HTTP_PUT);
    deleteWiFi->onRequest([&](AsyncWebServerRequest* request, JsonVariant& root) {
        Serial.print("[WiFiMgr] Deleting wifi: ");
        Serial.println(root["ssid"].as<String>());
        if(root["ssid"] == ""){
            request->send(400, "text/plain", "{\"error\":\"SSID can't be empty\"}");
        }else{
        deleteWiFiConnection(root["ssid"].as<String>());
        request->send(200, "text/plain", "{\"executed\":true}");
        }
    });
    _server->addHandler(deleteWiFi);

    //handle notFound for dns
    _server->onNotFound([](AsyncWebServerRequest *request){
        if(WiFi.getMode()==WIFI_MODE_APSTA){
            request->redirect("/wifi-mgr");
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });

    _server->serveStatic("/wifi-mgr", LittleFS, "/www/").setDefaultFile("wifi.html");
    _server->serveStatic("/assets/", LittleFS, "/www/assets/").setCacheControl("max-age=604800");
}

void TeslaWiFiManager::serverBegin(){
    _server->begin();
}

void TeslaWiFiManager::serverStop(){
    _server->end();
}

// connecting services
void TeslaWiFiManager::connect(WiFiCredentials network){
    WiFi.mode(WIFI_STA);
    WiFi.begin(network.ssid, network.password);
    _connectionTentative = true;
    Serial.printf("[WiFiMgr] Connecting to WiFi: %s\n", network.ssid);
}

//wait connection
bool TeslaWiFiManager::connectionWait(){
    while (millis() - _lms < 5000)
    {
        if(WiFi.status() == WL_CONNECTED) {
        _connectionTentative = false;
        return true;
        }
    }
    return false;
}


void TeslaWiFiManager::storeWiFiConnection(){

    Serial.println("New storing request\n");


    if(_toConnectNetwork.pref){
        Serial.println("Deleting prevous default connections\n");
        for (int i = 0; i < MAX_WIFI_NETWORKS; i++){
                _savedNetworks[i].pref = false;
        }
    }
    

    //SSID already exist, updating wifi setting
    for (size_t i = 0; i < MAX_WIFI_NETWORKS; i++)
    {
        if(_toConnectNetwork.ssid == _savedNetworks[i].ssid){
            Serial.println("[WiFiNgr] Updating existing network");
            _toConnectNetwork = _savedNetworks[i];
            writeNVS();
            return;
        }
    }
    
    //New SSID found a place where store it...
    int freeSpace = -1;
    for (int i = 0; i < MAX_WIFI_NETWORKS; i++)
    {
        if(_savedNetworks[i].isValid == false){
            _savedNetworks[i] = _toConnectNetwork;
            _savedNetworks[i].isValid = true;
            break;
        }
    }

    //if no space availble I hope some error appears before

    writeNVS();
}

void TeslaWiFiManager::deleteWiFiConnection(String ssid){
    int indexToRemove = -1;
    for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
        if (ssid == _savedNetworks[i].ssid) {
        indexToRemove = i;
        break;
        }
    }

    if(indexToRemove != -1){
        Serial.printf("[WiFiMgr] Deleting wifi: %s\n", ssid);
        resetWiFiCredential(_savedNetworks[indexToRemove]);
        writeNVS();
    } else {
        Serial.printf("[WiFiMgr] Unable to find wifi: %s\n", ssid);
    }

    
    return;
}

void TeslaWiFiManager::resetWiFiCredential(WiFiCredentials network){
    network.ssid="";
    network.password="";
    network.pref=false;
    network.isValid=false;
}

/* check if a configured WiFi is present on the scan list*/
bool TeslaWiFiManager::checkScannedNetworks(){
    if(_wifiNetworkFound < 1){
        return false;
    }

    for (int i = 0; i < _wifiNetworkFound; i++)
    {
        for (int y = 0; y < MAX_WIFI_NETWORKS; y++)
        {
            if(WiFi.SSID(i) == _savedNetworks[y].ssid){
                _toConnectNetwork = _savedNetworks[y];
                return true;
            }
        }
        
    }
    return false;
}


/* This function move the default connection at the top and fill gaps */
void TeslaWiFiManager::WiFiListOrder(){
    int defPosition = -1;
    for (size_t i = 0; i < MAX_WIFI_NETWORKS; i++)
    {
        if(_savedNetworks[i].isValid && _savedNetworks[i].pref){
            defPosition = i;
            break;
        }
    }
    
    if(defPosition > 0){
        _tmpNetwork = _savedNetworks[defPosition];
        for (size_t i = defPosition; i > 0 ; i--)
        {
            _savedNetworks[i] = _savedNetworks[i - 1];
        }
        _savedNetworks[0] = _tmpNetwork;
    }

    size_t writeIndex = 0;

    for (size_t readIndex = 0; readIndex < MAX_WIFI_NETWORKS; readIndex++) {
        if (_savedNetworks[readIndex].isValid) {
            if (writeIndex != readIndex) {
                _savedNetworks[writeIndex] = _savedNetworks[readIndex];
                resetWiFiCredential(_savedNetworks[readIndex]);
            }
            writeIndex++;
        }
    }

    resetWiFiCredential(_tmpNetwork);
}
void TeslaWiFiManager::prepareNvsPointer(int i){
  sprintf(ssid_key, "ssid_%d", i);
  sprintf(pass_key, "pass_%d", i);
  sprintf(pref_key, "pref_%d", i);
}


bool TeslaWiFiManager::initNVS(){
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

    if (_nvsHandler.begin(PREF_NAMESPACE, true)) {
      Serial.println("[WiFiMgr] Stored Wifi:");
      for (size_t i = 0; i < MAX_WIFI_NETWORKS; i++)
      {
        prepareNvsPointer(i);

        if(_nvsHandler.isKey(ssid_key) && (_nvsHandler.getString(ssid_key, "").length() > 0))
          {
          _savedNetworks[i].ssid = _nvsHandler.getString(ssid_key, "");
          _savedNetworks[i].password = _nvsHandler.getString(pass_key, "");
          _savedNetworks[i].pref = _nvsHandler.getBool(pref_key, "");
          _savedNetworks[i].isValid = true;
          Serial.printf("%d - %s\n",i, _savedNetworks[i].ssid);
          } else {
                resetWiFiCredential(_savedNetworks[i]);
          }
      }
      _nvsHandler.end();
      return true;
    }

    return false;
}

// return the number of valid network stored
int TeslaWiFiManager::storedWifi(){
  int validWiFi = 0;
  for (int i = 0; i < MAX_WIFI_NETWORKS; i++)
  {
    if(_savedNetworks[i].isValid){
      validWiFi++;
    }
  }
  return validWiFi;
  
}

void TeslaWiFiManager::writeNVS(){
  WiFiListOrder();
  _nvsHandler.begin(PREF_NAMESPACE, false);
  for (int i = 0; i < MAX_WIFI_NETWORKS ; i++)
  {
    prepareNvsPointer(i);
    if(_savedNetworks[i].isValid){
      _nvsHandler.putString(ssid_key,_savedNetworks[i].ssid);
      _nvsHandler.putString(pass_key,_savedNetworks[i].password);
      _nvsHandler.putBool(pref_key,_savedNetworks[i].pref);
    } else {
      
      if(_nvsHandler.isKey(ssid_key)){
        _nvsHandler.remove(ssid_key);
        if(_nvsHandler.isKey(pass_key)){
          _nvsHandler.remove(pass_key);
        }
        if(_nvsHandler.isKey(pref_key)){
          _nvsHandler.remove(pref_key);
        }
      }
    }
  }
  _nvsHandler.end();
  resetWiFiCredential(_toConnectNetwork);
}


void TeslaWiFiManager::printIP(){
    Serial.print("[WiFiMgr L] WiFi Connected, TeslaBoard IP address is: ");
    Serial.println(WiFi.localIP());
}