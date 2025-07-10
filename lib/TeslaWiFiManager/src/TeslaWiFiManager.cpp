#include "TeslaWiFiManager.h"

TeslaWiFiManager::TeslaWiFiManager(AsyncWebServer *server) : _server(server) {}


void TeslaWiFiManager::init(){
    if(!_routeInit){
        serverRouting(); 
        _routeInit = true;
    }
    
    while (_cycle != 1000000)
    {
        switch (_cycle)
            {
            case 0:
                //start the async wifi scan and read the json file
                startWiFiScan();
                
                if(!checkWiFiFile()){
                    Serial.println("[WiFiMgr] Wifi file no good, going to AP");
                    _cycle = 500;
                    break;
                }

                if(_json["stored"].size() == 0){
                    Serial.println("[WiFiMgr] No Wifi configured, going to AP");
                    _cycle = 500;
                    break;
                }
                waitWiFiScanCompleted();
                
                if(_wifiNetworkFound == 0){
                    Serial.println("[WiFiMgr] No Wifi founded (are you in the desert?), going to AP");
                    _cycle = 500;
                    break;
                }
                _cycle = 10;
                break;



                //get the stored wifi list
            case 10:
                    _wifiList = _json["stored"].as<JsonArray>();
                    WiFiListOrder();
                    _cycle = 11;
                    break;

            case 11: {
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
                        _cycle = 1000;
                    } else {
                        Serial.print("[WiFiMgr] Unable to connecto to any wifi, going in AP");
                        _cycle = 500;
                    }
                }
                break;



                //first connection
            case 100:
                break;

                //goes to AP
            case 500:
                waitWiFiScanCompleted();
                startCaptivePortal();
                _cycle = 510;
                break;
            case 510:
                while(WiFi.softAPIP() != IPAddress(192,168,4,1)){
                    ;
                }
                _cycle = 520;
                serverBegin();
                Serial.println("[WiFiMgr] Waiting for WiFi data");
                break;
            case 520:
                if(_newIncomingWiFi){
                    _newIncomingWiFi = false;
                    _cycle = 600;
                }
                break;

            case 600:
                stopCaptivePortal();
                _cycle = 605;
                break;
            case 605:
                connect(_incomingSSID,_incomingPSW);
                _cycle = 610;
                _lms = millis();
                break;
            case 610:
                if(connectionWait()){
                    _cycle = 700;
                }
                else {
                    startWiFiScan();
                    _cycle = 620;
                }
                break;
            case 620:
                waitWiFiScanCompleted();
                _cycle = 500;
                break;
            case 700:
                storeWiFiConnection();
                _cycle = 1000;
                break;

            case 1000:
                Serial.println(WiFi.localIP());
                _cycle = 1001;
                break;    
            case 1001:
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

void TeslaWiFiManager::startWiFiScan(){
    Serial.println("[WiFiMgr] Scan in progress...");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
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

void TeslaWiFiManager::startCaptivePortal(){
    WiFi.mode(WIFI_AP);
    WiFi.softAP("TeslaBoard", "123456789");
    WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
    _dnsServer.start(53, "*",IPAddress(192,168,4,1));
    _dnsServerActive = true;
}

void TeslaWiFiManager::stopCaptivePortal(){
    _dnsServer.stop();
    _dnsServerActive = false;
    Serial.println("[WiFiMgr] stop dns");
    serverStop();
    Serial.println("[WiFiMgr] stop server");
    WiFi.softAPdisconnect(true);
    Serial.println("[WiFiMgr] stop ap");
    WiFi.mode(WIFI_OFF);
    Serial.println("[WiFiMgr] stop wifi");
    Serial.println("[WiFiMgr] AP closed");    
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
        if(WiFi.mode(WIFI_MODE_AP)){
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

void TeslaWiFiManager::loop(){

    if(WiFi.status() != WL_CONNECTED){
        Serial.println("[WiFiMgr] WiFi connection lost.");
    }
}

// File Manager
void TeslaWiFiManager::readWiFiFile(){
    _json.clear();
    _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_READ);
    deserializeJson(_json,_fileReader);    
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
        _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_WRITE);
        serializeJson(_json,_fileReader);
        _fileReader.close();
        _incomingSSID="";
        _incomingPSW="";
        _incomingDefault="";
        _newIncomingWiFi=false;
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
    _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_WRITE);
    serializeJson(_json,_fileReader);
    _fileReader.close();
    return;
}

void TeslaWiFiManager::deleteWiFiConnection(String ssid){

    if(!checkWiFiFile()){
        return;
    }

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

    _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_WRITE);
    _fileReader.close();
    return;
}

bool TeslaWiFiManager::checkWiFiFile(){
    _json.clear();
    _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_READ);

    if (_fileReader) {
        DeserializationError desErr = deserializeJson(_json, _fileReader);
        _fileReader.close();
        if(desErr){
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