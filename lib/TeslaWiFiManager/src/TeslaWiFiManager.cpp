#include "TeslaWiFiManager.h"

TeslaWiFiManager::TeslaWiFiManager(AsyncWebServer *server) : _server(server) {}

//startup cycle
void TeslaWiFiManager::init(){
    if(!_routeInit){
        serverRouting(); 
        WiFi.mode(WIFI_STA); //turn on wifi
        _routeInit = true;
    }
    
    while (1)
    {
        switch (_cycle)
            {
            case INIT:
                

                if(!checkWiFiFile()){
                    Serial.println("[WiFiMgr] Wifi file no good, going to AP");
                    _cycle = START_SCAN;
                    break;
                } else if(_json["stored"].size() == 0){
                    Serial.println("[WiFiMgr] No Wifi configured, going to AP");
                    _cycle = START_SCAN;
                    break;
                }
                
                _cycle = STORED_ORDERING;
                break;

                //get the stored wifi list
            case STORED_ORDERING:
                    _wifiList = _json["stored"].as<JsonArray>();
                    WiFiListOrder();
                    _cycle = CONNECT_TO_STORED_WIFI;
                    break;

            case CONNECT_TO_STORED_WIFI: {
                    bool connected = false;
                    for (JsonObject item : _wifiList) {
                        connect(item["ssid"],item["psw"]);
                        _lms = millis();
                        if(connectionWait()){
                            connected = true;
                            break;
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
                //start the async wifi scan and read the json file
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
                connect(_SSIDtoConnect,_PSWtoConnect);
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
            //_loopCycle = L_BACK_TO_STA_MODE;
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
        _storedWifi = _json["stored"].size();
        
        if(_skipThisNetwork){
            _skipThisNetwork = false;
            _StoredWiFiPointer++;
        }

        if(_StoredWiFiPointer >= _storedWifi){
            //tested all stored connections
            //but something get wrong...
            //try to perfomr a new rescan
            Serial.println("[WiFiMgr L] Tried to connect on each stored WiFi without success, perform a new scan..");
            _loopCycle = L_DISCONNECT_WIFI;
            break;
        }

        if(WiFi.SSID(_SSIDWiFiPointer) != _json["stored"][_StoredWiFiPointer]["ssid"].as<String>()){
            Serial.print("[WiFiMgr L] Comparing SSID: ");
            Serial.print(WiFi.SSID(_SSIDWiFiPointer));
            Serial.print(" with the stored SSID: ");
            Serial.println(_json["stored"][_StoredWiFiPointer]["ssid"].as<String>());

            _SSIDWiFiPointer++;
            if(_SSIDWiFiPointer >= _wifiNetworkFound){
                _SSIDWiFiPointer = 0;
                _StoredWiFiPointer++;
            }
        } else {

            Serial.print("[WiFiMgr L] Comparing SSID: ");
            Serial.print(WiFi.SSID(_SSIDWiFiPointer));
            Serial.print(" with the stored SSID: ");
            Serial.println(_json["stored"][_StoredWiFiPointer]["ssid"].as<String>());

            //try to connect to the stored and founded SSID
            connect(_json["stored"][_StoredWiFiPointer]["ssid"].as<String>(),_json["stored"][_StoredWiFiPointer]["psw"].as<String>());
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
        _cycle = AP_STOP;
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

        for (JsonObject wifi : _json["stored"].as<JsonArray>()) {
            JsonObject newObj = wifiConfigured.add<JsonObject>();
            newObj["ssid"] = wifi["ssid"]; 
            newObj["default"] = wifi["default"];
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
            request->send(400, "text/plain", "no ssid");
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
        if(root["ssid"] == ""){
            request->send(400, "text/plain", "{\"error\":\"No ssid found\"}");
        }
        _incomingSSID = root["ssid"].as<String>();
        _incomingPSW = root["psw"].as<String>();
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

// File Manager
void TeslaWiFiManager::readWiFiFile(){
    _json.clear();
    _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_READ);
    deserializeJson(_json,_fileReader);    
}

void TeslaWiFiManager::writeWiFiFile(){
    _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_WRITE);
    serializeJson(_json,_fileReader); 
    _fileReader.close();   
    resetIncomingParameters();
}

void TeslaWiFiManager::storeWiFiConnection(){

    if(!checkWiFiFile()){
        //we are going to recreate the file
        _json.clear();
        Serial.println("[WiFiMgr] Generating a new wifi file");
        JsonArray wifiArray = _json["stored"].to<JsonArray>();
        JsonObject wifi = wifiArray.add<JsonObject>();
        wifi["ssid"] = _incomingSSID;
        wifi["psw"] = _incomingPSW;
        wifi["default"] = _incomingDefault;
        writeWiFiFile();
        return;
    }
    
    JsonArray stored = _json["stored"].as<JsonArray>();
    bool found = false;
    for (JsonObject item : stored) {
        if (_incomingSSID == item["ssid"].as<String>()) {
            found = true;
            Serial.print("[WiFiMgr] Replacing existing WiFi network: ");
            item["psw"] = _incomingPSW;
            item["default"] = _incomingDefault;
        } else if (_incomingDefault) {
            item["default"] = false;
        }
    }

    if(!found){
        Serial.print("[WiFiMgr] Adding a new WiFi network: ");
        Serial.println(_incomingSSID);
        JsonObject incomingWiFi = stored.add<JsonObject>();
        incomingWiFi["ssid"] = _incomingSSID;
        incomingWiFi["psw"] = _incomingPSW;
        incomingWiFi["default"] = _incomingDefault;
    }
    writeWiFiFile();
    return;
}

void TeslaWiFiManager::deleteWiFiConnection(String ssid){

    int indexToRemove = -1;
    for (int i = 0; i < _json["stored"].size(); i++) {
        if (ssid == _json["stored"][i]["ssid"].as<String>()) {
        indexToRemove = i;
        break;
        }
    }

    if(indexToRemove != -1){
        _json["stored"].remove(indexToRemove);
    }

    writeWiFiFile();
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

    JsonArray stored = _json["stored"].as<JsonArray>();
    for (int i = 0; i < _wifiNetworkFound; i++)
    {
        for(JsonObject item : stored){
            if(WiFi.SSID(i) == item["ssid"].as<String>()){
                _SSIDtoConnect = item["ssid"].as<String>();
                _PSWtoConnect = item["psw"].as<String>();
                return true;
            }
        }
    }
    return false;
}

bool TeslaWiFiManager::checkWiFiFile(){
    _json.clear();
    _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_READ);

    if (_fileReader) {
        DeserializationError desErr = deserializeJson(_json, _fileReader);
        _fileReader.close();
        if(desErr){
            Serial.print(desErr.c_str());
            Serial.println("[WiFiMgr] Error during deserialization of wifi file");
            return false;
        } else {
            if (!_json["stored"].is<JsonArray>()) {
                Serial.println("[WiFiMgr] stored is not an array");
                return false;
            }
        }
    } else {
        Serial.println("[WiFiMgr] Unable to read wifi file");
        return false;
    }
    return true;
}

/* This function move the default connection at the top */
void TeslaWiFiManager::WiFiListOrder(){
    _wifiList = _json["stored"].as<JsonArray>();

    int size = _wifiList.size();
    int defPos = -1;
    if(size==1){
        return;
    }

    for (int i = 0; i < size; i++)
    {
        if(_wifiList[i]["default"]){
            defPos = i;
            break;
        }
    }
    
    if(defPos > 0){
        JsonObject tmp;
        tmp.set(_wifiList[0]);
        _wifiList[0].set(_wifiList[defPos]);
        _wifiList[defPos].set(tmp);
    }

}