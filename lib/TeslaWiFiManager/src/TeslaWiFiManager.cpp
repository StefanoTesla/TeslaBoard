#include "TeslaWiFiManager.h"

TeslaWiFiManager::TeslaWiFiManager(AsyncWebServer *server) : _server(server) {}


void TeslaWiFiManager::init(){
    if(!_routeInit){
        Serial.println("Setup routes");
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
                waitWiFiScanCompleted();

                if(!checkWiFiFile()){
                    _cycle = 500;
                }

                _cycle = 10;
                break;

                //first connection
            case 10:
                readWiFiFile();
                break;

                //first connection
            case 100:
                break;

                //goes to AP
            case 500:
                startCaptivePortal();
                _cycle = 510;
                break;
            case 510:
                while(WiFi.softAPIP() != IPAddress(192,168,4,1)){
                    ;
                }
                _cycle = 520;
                serverBegin();
                Serial.println("Waiting for WiFi data");
                break;
            case 520:
                if(_newIncomingWiFi){
                    _newIncomingWiFi = false;
                    _cycle = 600;
                }
                break;

            case 600:
                Serial.print("Connecting to: ");
                Serial.print(_incomingSSID);
                stopCaptivePortal();
                _cycle = 605;
                break;
            case 605:
                connect(_incomingSSID,_incomingPSW);
                _cycle = 610;
                _lms = millis();
                break;
            case 610:
                if(WiFi.status() == WL_CONNECTED) {
                    Serial.println("Connected");
                    _cycle = 700;
                }

                if(millis() - _lms > 10000){
                    Serial.println("Unable to connect");
                    startWiFiScan();
                    _cycle = 620;
                }
                break;
            case 620:
                waitWiFiScanCompleted();
                _cycle = 500;
                break;
            case 700:
                storeWiFiSetting();
                _cycle = 800;
                break;

            case 800:
                Serial.println(WiFi.localIP());
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
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.scanNetworks(true);
    _scanInProgress = true;
}

void TeslaWiFiManager::waitWiFiScanCompleted(){
    Serial.println();
    Serial.print("Scan in progress.");
    while(WiFi.scanComplete() < 0  ){
        ;
    }
    _wifiNetworkFound = WiFi.scanNetworks();
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
    serverStop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("AP closed");    
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
        JsonArray wifi = doc["wifi"].to<JsonArray>();
        for (size_t i = 0; i < _wifiNetworkFound; i++)
        {
            wifi[i]["ssid"] = WiFi.SSID(i);
            wifi[i]["rssi"] = WiFi.RSSI(i);
            if(WiFi.encryptionType(i) == 0){
                wifi[i]["enc"] = "open";
            } else {
                wifi[i]["enc"] = "psw";
            }
        }
        response->setLength();
        request->send(response);
    });

    //perform a new scan
    _server->on("/wifi-api/force-scan", HTTP_GET, [&](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "ok");
    });


    //new wifi to be stored
    AsyncCallbackJsonWebHandler* incomingWiFi = new AsyncCallbackJsonWebHandler("/wifi-api/new-wifi");
    incomingWiFi->setMethod(HTTP_POST | HTTP_PUT);
    incomingWiFi->onRequest([&](AsyncWebServerRequest* request, JsonVariant& root) {
        serializeJson(root,Serial);
        if(root["ssid"] == ""){
            request->send(400, "text/plain", "no ssid");
        }
        _incomingSSID = root["ssid"].as<String>();
        _incomingPSW = root["psw"].as<String>();
        _newIncomingWiFi = true;
        request->send(200, "text/plain", "ok");
    });
    _server->addHandler(incomingWiFi);
    //handle notFound for dns
    _server->onNotFound([](AsyncWebServerRequest *request){
        if(WiFi.mode(WIFI_MODE_AP)){
            request->redirect("/wifi-mgr/");
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });

    _server->serveStatic("/assets/", LittleFS, "/www/assets/").setCacheControl("max-age=604800");
    _server->serveStatic("/langs/", LittleFS, "/www/langs/").setCacheControl("max-age=604800");
}

void TeslaWiFiManager::serverBegin(){
    _server->begin();
}

void TeslaWiFiManager::serverStop(){
    _server->end();
}

void TeslaWiFiManager::connect(String SSID,String Password){
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, Password);
    Serial.print("Connecting to WiFi ..");
}

void TeslaWiFiManager::loop(){

    if(WiFi.status() != WL_CONNECTED){
        //disconnesso :(
    }
}


// File Manager
void TeslaWiFiManager::readWiFiFile(){
    _json.clear();
    _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_READ);
    deserializeJson(_json,_fileReader);
    JsonArray wifi = _json["wifi"].as<JsonArray>();

    
}

void TeslaWiFiManager::storeWiFiSetting(){

    if(!checkWiFiFile()){
        //we are going to recreate the file
        _json.clear();
        Serial.println("Generating a new wifi file");
        JsonArray wifi = _json.createNestedArray("wifi");
        JsonObject wifi_0 = wifi.createNestedObject();
        wifi_0["ssid"] = _incomingSSID;
        wifi_0["psw"] = _incomingPSW;
        wifi_0["default"] = _incomingDefault;
        _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_WRITE);
        serializeJson(_json,_fileReader);
        _fileReader.close();
        return;
    }
    
    JsonArray wifi = _json["wifi"].as<JsonArray>();
    bool found = false;
    for (JsonObject item : wifi) {
        if (_incomingSSID == item["ssid"].as<String>()) {
            found = true;
            item["psw"] = _incomingPSW;
            item["default"] = _incomingDefault;
        } else if (_incomingDefault) {
            item["default"] = false;
        }
    }

    if(!found){
        Serial.print("Adding a new WiFi network: ");
        Serial.println(_incomingSSID);
        JsonObject incomingWiFi = wifi.createNestedObject();
        incomingWiFi["ssid"] = _incomingSSID;
        incomingWiFi["psw"] = _incomingPSW;
        incomingWiFi["default"] = _incomingDefault;
    }
    _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_WRITE);
    serializeJson(_json,_fileReader);
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
            Serial.println("Error during deserialization of wifi file");
            return false;
        } else {
            if (!_json["wifi"].is<JsonArray>()) {
                Serial.println("Wifi is not an array");
                return false;
            }
        }
    } else {
        Serial.println("Unable to read wifi file");
        return false;
    }
    return true;
}