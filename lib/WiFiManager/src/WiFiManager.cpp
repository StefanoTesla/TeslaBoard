#include "WiFiManager.h"

WiFiManager::WiFiManager(AsyncWebServer *server) : _server(server) {}

void WiFiManager::init(){

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
        break;
    
    default:
        break;
    }


}


void WiFiManager::startWiFiScan(){
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.scanNetworks(true);
    _scanInProgress = true;
}

void WiFiManager::waitWiFiScanCompleted(){
    _lastMillis = millis();
    Serial.println();
    Serial.print("Scan in progress.");
    while(WiFi.scanComplete() < 0  ){
        if(millis() - _lastMillis > 500){
            Serial.print(".");
        }
        
    }
    _scanInProgress = false;
}

void WiFiManager::readWiFiFile(){
        //in the meanwhile check if wifi file exits
    _fileReader = LittleFS.open("/cfg/wifi.txt", FILE_READ);

    if (_fileReader) {
        DeserializationError desErr = deserializeJson(_json, _fileReader);
        if(desErr){
            _desWiFiFileError = true;
        }
    } else {
        _missingWiFiFile = true;
    }
    _fileReader.close();

}

void WiFiManager::startCaptivePortal(){
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
    _dnsServer.start(53, "*",IPAddress(192,168,4,1));
}



void WiFiManager::loop(){

    if(WiFi.status() != WL_CONNECTED){
        //disconnesso :(
    }
}