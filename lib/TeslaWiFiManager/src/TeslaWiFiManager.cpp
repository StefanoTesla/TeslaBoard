#include "TeslaWiFiManager.h"

TeslaWiFiManager::TeslaWiFiManager(AsyncWebServer *server) : _server(server) {}

//startup cycle
void TeslaWiFiManager::init(){
    if(!_routeInit){
        initNVS();
        initWiFiStruct();
        serverRouting(); 
        WiFi.mode(WIFI_STA); //turn on wifi
        _routeInit = true;
    }
    
    while (1)
    {
        switch (_cycle)
            {
            case INIT:
                //check if a wifi is valid
                for (int i = 0; i < MAX_WIFI_NETWORKS ; i++)
                {
                    if(savedNetworks[i].isValid){
                        _cycle = CONNECT_TO_STORED_WIFI;
                        break;
                    }
                }

                Serial.println("[WiFiMgr] No stored Wifi founds, going to AP");
                
                _cycle = BEFORE_START_AP;
                break;

            case CONNECT_TO_STORED_WIFI: {
                    bool connected = false;
                    for (size_t i = 0; i < MAX_WIFI_NETWORKS; i++)
                    {
                        if(savedNetworks[i].isValid){
                        connect(savedNetworks[i].ssid,savedNetworks[i].password);
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
                        Serial.println("[WiFiMgr] Unable to connecto to any wifi, going in AP");
                        _cycle = BEFORE_START_AP;
                    }
                }
                break;

            case BEFORE_START_AP:
                WiFi.disconnect();
                WiFi.scanDelete();
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
                connect(_incomingSSID,_incomingPSW);
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
                Serial.println("[WiFiMgr] TeslaBoard IP address is: ");
                Serial.print(WiFi.localIP());
                Serial.println("");
                return;
                break;             
            default:
                Serial.println("BEVI MENO DIOCAN");
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
            Serial.println("[WiFiMgr L] WiFi connection lost.");
            Serial.println("[WiFiMgr L] Shut down the WiFi");
            _loopCycle = L_DISCONNECT_WIFI;
            _lms = millis();
        }
        break;

    case L_DISCONNECT_WIFI:
        WiFi.disconnect(true); //workaround to make scan working
        _loopCycle = L_SHUTDOWN_WIFI;
        _lms = millis();
        break;

    case L_SHUTDOWN_WIFI:
        if(WiFi.getMode() == WIFI_OFF){
            Serial.println("[WiFiMgr L] set the WiFi on station mode");
            WiFi.mode(WIFI_AP_STA); //workaround to make scan working
            _loopCycle = L_BACK_TO_STA_MODE;
        }
        break;

    case L_BACK_TO_STA_MODE:
        if(WiFi.getMode() == WIFI_AP_STA){
            Serial.println("[WiFiMgr L] WiFi is on ap and station mode");
            _lms = millis();
            _loopCycle = L_START_SCAN;
        }
        break;

    case L_START_SCAN:
        startWiFiScan();
        _loopCycle = L_WAIT_SCAN;
        break;
            
    case L_WAIT_SCAN:
        Serial.print(".");
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

        if(savedNetworks[_StoredWiFiPointer].isValid && (WiFi.SSID(_SSIDWiFiPointer) != savedNetworks[_StoredWiFiPointer].ssid)){
            Serial.print("[WiFiMgr L] Comparing SSID: ");
            Serial.print(WiFi.SSID(_SSIDWiFiPointer));
            Serial.print(" with the stored SSID: ");
            Serial.println(savedNetworks[_StoredWiFiPointer].ssid);

            _SSIDWiFiPointer++;
            if(_SSIDWiFiPointer >= _wifiNetworkFound){
                _SSIDWiFiPointer = 0;
                _StoredWiFiPointer++;
            }
        } else {

            Serial.print("[WiFiMgr L] Comparing SSID: ");
            Serial.print(WiFi.SSID(_SSIDWiFiPointer));
            Serial.print(" with the stored SSID: ");
            Serial.println(savedNetworks[_StoredWiFiPointer].ssid);

            //try to connect to the stored and founded SSID
            connect(savedNetworks[_StoredWiFiPointer].ssid,savedNetworks[_StoredWiFiPointer].password);
            _lms = millis();
            _loopCycle = L_WAIT_CONNECTION_TO_STORED_WIFI;
        }
        break;

    case L_WAIT_CONNECTION_TO_STORED_WIFI:
        if(WiFi.status() == WL_CONNECTED){
            Serial.println("[WiFiMgr L] WiFi Connected, Enjoy!");
            _loopCycle = L_LOOP;
            _SSIDWiFiPointer = 0;
            _StoredWiFiPointer = 0;
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
    Serial.println("[WiFiMgr] Scan in progress...");
    WiFi.scanDelete();
    WiFi.scanNetworks(true);
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
    Serial.println("[WiFiMgr] stop dns");
    _dnsServer.stop();
    _dnsServerActive = false;
    Serial.println("[WiFiMgr] stop server");
    serverStop();
    Serial.println("[WiFiMgr] stop ap");
    WiFi.softAPdisconnect(true);
    Serial.println("[WiFiMgr] wifi off");
    WiFi.mode(WIFI_OFF);
    Serial.println("[WiFiMgr] AP closed");    
}

void TeslaWiFiManager::APLoop(){
    //new connection request
    if(_newIncomingWiFi){
        Serial.println("deogan");
        _cycle = AP_STOP;
        return;
    }

    // background scanning
    // if a configured WiFi is found try to connect to..
    //wait 30s
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

    _server->on("/wifi-mgr/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/www/wifi.html.gz", "text/html");
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });

    //wifi list
    _server->on("/wifi-api/wifi-list", HTTP_GET, [&](AsyncWebServerRequest *request){
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
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

        JsonArray wifiConfigured = doc["stored"].to<JsonArray>();
        for (size_t i = 0; i < MAX_WIFI_NETWORKS; i++)
        {
            if(savedNetworks[i].isValid){
                JsonObject newObj = wifiConfigured.add<JsonObject>();
                newObj["ssid"] = savedNetworks[i].ssid; 
                newObj["default"] = savedNetworks[i].pref;
            }
        }

        response->setLength();
        request->send(response);
    });

    //perform a new scan
    _server->on("/wifi-api/force-scan", HTTP_GET, [&](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "{\"executed\":true}");
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
        if(root["psw"].as<String>().length() < 8){
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

        
        _incomingSSID = root["ssid"].as<String>();
        _incomingPSW = root["psw"].as<String>();
        _incomingDefault = root["default"].as<bool>();
        _newIncomingWiFi = true;
        request->send(200, "text/plain", "{\"executed\":true}");
    });
    _server->addHandler(incomingWiFi);

    //new wifi to be stored
    AsyncCallbackJsonWebHandler* addWiFi = new AsyncCallbackJsonWebHandler("/wifi-api/add-wifi");
    addWiFi->setMethod(HTTP_POST | HTTP_PUT);
    addWiFi->onRequest([&](AsyncWebServerRequest* request, JsonVariant& root) {
        Serial.print("[WiFiMgr] New wifi to be added: ");
        Serial.println(root["ssid"].as<String>());
        if(root["ssid"].as<String>() == ""){
            request->send(400, "text/plain", "{\"error\":\"No ssid found\"}");
            return;
        }

        if(root["ssid"].as<String>().length() > 32){
            request->send(400, "text/plain", "{\"error\":\"SSID too long\"}");
            return;
        }
        if(root["psw"].as<String>().length() < 8){
            request->send(400, "text/plain", "{\"error\":\"Password too short\"}");
            return;
        }
        if(root["psw"].as<String>().length() > 63){
            request->send(400, "text/plain", "{\"error\":\"Password too long\"}");
            return;
        }

        if(storedWifi() >= MAX_WIFI_NETWORKS){
            request->send(400, "text/plain", "{\"error\":\"no space, delete some wifi\"");
            return;
        }

        _incomingSSID = root["ssid"].as<String>();
        _incomingPSW = root["pws"].as<String>();
        _incomingDefault = root["default"].as<bool>();
        
        storeWiFiConnection();
        request->send(200, "text/plain", "{\"executed\":true}");
    });
    _server->addHandler(addWiFi);

    //delete wifi
    AsyncCallbackJsonWebHandler* deleteWiFi = new AsyncCallbackJsonWebHandler("/wifi-api/delete-wifi");
    deleteWiFi->setMethod(HTTP_POST | HTTP_PUT);
    deleteWiFi->onRequest([&](AsyncWebServerRequest* request, JsonVariant& root) {
        Serial.print("[WiFiMgr] Deleting wifi: ");
        Serial.println(root["ssid"].as<String>());
        if(root["ssid"] == ""){
            request->send(400, "text/plain", "{\"error\":\"No ssid found\"}");
        }else{
        deleteWiFiConnection(root["ssid"].as<String>());
        request->send(200, "text/plain", "{\"executed\":true}");
        }
    });
    _server->addHandler(deleteWiFi);

    //handle notFound for dns
    _server->onNotFound([](AsyncWebServerRequest *request){
        if(WiFi.mode(WIFI_MODE_APSTA)){
            request->redirect("/wifi-mgr/");
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });

    _server->serveStatic("/assets/", LittleFS, "/www/assets/").setCacheControl("max-age=604800");
}

void TeslaWiFiManager::serverBegin(){
    _server->begin();
}

void TeslaWiFiManager::serverStop(){
    _server->end();
}

// connecting services
void TeslaWiFiManager::connect(String SSID,String Password){
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, Password);
    Serial.print("[WiFiMgr] Connecting to WiFi: ");
    Serial.println(SSID);
}

//wait connection

bool TeslaWiFiManager::connectionWait(){
    while (millis() - _lms < 5000)
    {
        if(WiFi.status() == WL_CONNECTED) {
        return true;
        }
    }
    return false;
}


void TeslaWiFiManager::storeWiFiConnection(){

    Serial.println("New storing request\n");
    int freeSpace = -1;
    for (int i = 0; i < MAX_WIFI_NETWORKS; i++)
    {
        if(savedNetworks[i].isValid == false){
            freeSpace = i;
            break;
        }
    }

    Serial.printf("Empty place found in %d\n", freeSpace);
    
    if(freeSpace > -1){

        Serial.printf("Hidrating data for ssid %s\n", _incomingSSID);
        savedNetworks[freeSpace].ssid = _incomingSSID;
        savedNetworks[freeSpace].password = _incomingPSW;
        savedNetworks[freeSpace].pref = _incomingDefault;
        savedNetworks[freeSpace].isValid = true;
    } else {
        Serial.println("no space availble uhu\n");
        return;
    }

    if(_incomingDefault){
        Serial.println("Deleting prevous default connections\n");
        for (int i = 0; i < MAX_WIFI_NETWORKS; i++)
        {
            if(i != freeSpace){
                savedNetworks[i].pref = false;
            }
        }
    }

    Serial.printf("Let's go writing NVS\n");
    WriteNVS();

}


void TeslaWiFiManager::deleteWiFiConnection(String ssid){

    int indexToRemove = -1;
    for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
        if (ssid == savedNetworks[i].ssid) {
        indexToRemove = i;
        break;
        }
    }

    if(indexToRemove != -1){
        Serial.println("adios amigooo");
        savedNetworks[indexToRemove].ssid ="";
        savedNetworks[indexToRemove].password ="";
        savedNetworks[indexToRemove].pref =false;
        savedNetworks[indexToRemove].isValid =false;
    }

    WriteNVS();
    return;
}

void TeslaWiFiManager::resetIncomingParameters(){
    _incomingSSID="";
    _incomingPSW="";
    _incomingDefault="";
    _newIncomingWiFi=false;
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
            if(WiFi.SSID(i) == savedNetworks[y].ssid){
                _incomingSSID = savedNetworks[y].ssid;
                _incomingPSW = savedNetworks[y].password;
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
        if(savedNetworks[i].isValid && savedNetworks[i].pref){
            defPosition = i;
            break;
        }
    }
    
    if(defPosition > 0){
        Serial.println("default not in the right position moving to the top");
        tmpNetwork = savedNetworks[defPosition];
        for (size_t i = defPosition; i > 0 ; i--)
        {
            savedNetworks[i] = savedNetworks[i - 1];
        }
        savedNetworks[0] = tmpNetwork;
    }

    size_t writeIndex = 0;

    for (size_t readIndex = 0; readIndex < MAX_WIFI_NETWORKS; readIndex++) {
        if (savedNetworks[readIndex].isValid) {
            if (writeIndex != readIndex) {
                Serial.printf("\nhole founded, moving %d in the right position %d", readIndex,writeIndex);
                savedNetworks[writeIndex] = savedNetworks[readIndex];
                savedNetworks[readIndex].ssid = "";
                savedNetworks[readIndex].password = "";
                savedNetworks[readIndex].pref = false;
                savedNetworks[readIndex].isValid = false;
            }
            writeIndex++;
        }
    }
}




