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
                readWiFiFile();
                waitWiFiScanCompleted();

                if(_missingWiFiFile || _desWiFiFileError){
                    _cycle = 500;
                }

                //try to connect to a wifi
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
                    Serial.print("-");
                }
                _cycle = 520;
                serverBegin();
                Serial.println("Waiting for WiFi data");
                break;
            case 520:

                break;

            case 600:
                break;
            
            default:
                break;
        }

            if(_dnsServerActive){
                _dnsServer.processNextRequest();
                Serial.println("#");
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

void TeslaWiFiManager::readWiFiFile(){
        //in the meanwhile check if wifi file exits
    _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_READ);

    if (_fileReader) {
        DeserializationError desErr = deserializeJson(_json, _fileReader);
        if(desErr){
            Serial.println("Error during deserialization of wifi file");
            _desWiFiFileError = true;
        }
    } else {
        Serial.println("Unable to read wifi file");
        _missingWiFiFile = true;
    }
    _fileReader.close();

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
    Serial.println("AP closed");    
}

void TeslaWiFiManager::setStaticIP(IPAddress ip, IPAddress gw,IPAddress sn){

}

void TeslaWiFiManager::serverRouting(){
    //homepage
    _server->on("/wifi-mgr/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "✅ Accesso consentito solo su AP");
    });

    //wifi list
    _server->on("/wifi-mgr/api/wifi-list", HTTP_GET, [&](AsyncWebServerRequest *request){
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


    //new wifi to be stored
    _server->on


    //handle notFound for dns
    _server->onNotFound([](AsyncWebServerRequest *request){
        if(WiFi.mode(WIFI_MODE_AP)){
            request->redirect("/wifi-mgr/");
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });
}


void TeslaWiFiManager::serverBegin(){
    _server->begin();
}
void TeslaWiFiManager::serverStop(){
    _server->end();
}


void TeslaWiFiManager::loop(){

    if(WiFi.status() != WL_CONNECTED){
        //disconnesso :(
    }
}