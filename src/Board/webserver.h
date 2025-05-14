#ifndef BOARD_WEBSERVER
#define BOARD_WEBSERVER
#include "libraries.h"

void boardWebServer(){

    server.on("/api/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["locale"] = Global.config.language.locale;

        JsonObject define = doc["define"].to<JsonObject>();
        define["dome"] = false;
        define["switch"] = false;
        define["coverc"] = false;
        #ifdef DOME
            define["dome"] = true;
        #endif
        #ifdef SWITCH
            define["switch"] = true;
        #endif
        #ifdef COVER_CALIBRATOR
            define["coverc"] = true;
        #endif
        doc["version"] = _SW_VERSION_
        response->setLength();
        request->send(response);
    });


    server.on("/api/board/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["locale"] = Global.config.language.locale;

        JsonObject wifi = doc["wifi"].to<JsonObject>();
        wifi["reconTime"] = Global.config.wifi.reconnection.intervall;


        JsonObject address = doc["address"].to<JsonObject>();
        address["enStaticIP"] = Global.config.wifi.ip.enable;

        JsonObject ip = address["staticIp"].to<JsonObject>();
        ip["0"]= Global.config.wifi.ip.ip[0];
        ip["1"]= Global.config.wifi.ip.ip[1];
        ip["2"]= Global.config.wifi.ip.ip[2];
        ip["3"]= Global.config.wifi.ip.ip[3];

        JsonObject gw = address["staticGateway"].to<JsonObject>();
        gw["0"]= Global.config.wifi.ip.gw[0];
        gw["1"]= Global.config.wifi.ip.gw[1];
        gw["2"]= Global.config.wifi.ip.gw[2];
        gw["3"]= Global.config.wifi.ip.gw[3];


        JsonObject sub = address["staticSubnet"].to<JsonObject>();
        sub["0"]= Global.config.wifi.ip.sub[0];
        sub["1"]= Global.config.wifi.ip.sub[1];
        sub["2"]= Global.config.wifi.ip.sub[2];
        sub["3"]= Global.config.wifi.ip.sub[3];


        response->setLength();
        request->send(response);
    });


    server.on("/api/board/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        JsonObject wifi = doc["wifi"].to<JsonObject>();
        wifi["uptime"] = Global.config.wifi.upTime.minutes;
        wifi["ssid"] = WiFi.SSID();
        wifi["db"] = WiFi.RSSI();
        wifi["ip"] = WiFi.localIP();
        wifi["sub"] = WiFi.subnetMask();
        wifi["mac"] = WiFi.macAddress();

        JsonObject memo = doc["memory"].to<JsonObject>();
        memo["heapSize"] = ESP.getHeapSize();
        memo["freeHeap"] = ESP.getFreeHeap();
        memo["minHeap"] = ESP.getMinFreeHeap();

        JsonObject board = doc["board"].to<JsonObject>();
        board["uptime"] = Global.config.esp32.upTime.minutes;
        board["cpu"] = ESP.getChipModel();
        board["cores"] = ESP.getChipCores();
        board["rev"] = ESP.getChipRevision();
        board["freq"] = ESP.getCpuFreqMHz();
        

        JsonArray pwm = doc["pwm"].to<JsonArray>();
        for (size_t i = 0; i < 16; i++)
        {
            pwm.add(Global.pwm[i].type);
        }
        
        response->setLength();
        request->send(response);
    });


    AsyncCallbackJsonWebHandler* boardConfigHandler = new AsyncCallbackJsonWebHandler("/api/board/cfg");

    boardConfigHandler->setMethod(HTTP_POST | HTTP_PUT);
    boardConfigHandler->onRequest([](AsyncWebServerRequest* request, JsonVariant& json) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            JsonArray err = doc["errors"].to<JsonArray>();
            bool error = false;
            bool reboot = false;
            bool enable = false;
            
            serializeJson(json, Serial);

            if(!json["locale"].is<String>()){
                error = true;
                doc["errors"].add("Wrong Locale");
            }
            if(!json["wifi"]["reconTime"].is<unsigned int>()){
                error = true;
                doc["errors"].add("Wrong reconnection time");
            }
            JsonObject address = json["address"];
            if(!address["enStaticIP"].is<bool>()){
                doc["errors"].add("Wrong Static IP enable");
                error = true;
            } else {
                enable = address["enStaticIP"];
            }

            JsonObject address_staticIp = address["staticIp"];
            JsonObject address_staticGateway = address["staticGateway"];
            JsonObject address_staticSubnet = address["staticSubnet"];
            if(enable){
                
                if(!address_staticIp["0"].is<unsigned int>() || address_staticIp["0"].as<int>() > 255){
                    error = true;
                    doc["errors"].add("Wrong IP [0]");
                }
                if(!address_staticIp["1"].is<unsigned int>() || address_staticIp["1"].as<int>() > 255){
                    error = true;
                    doc["errors"].add("Wrong IP [1]");
                }
                if(!address_staticIp["2"].is<unsigned int>() || address_staticIp["2"].as<int>() > 255 ){
                    error = true;
                    doc["errors"].add("Wrong IP [2]");
                }
                if(!address_staticIp["3"].is<unsigned int>() || address_staticIp["3"].as<int>() > 255){
                    error = true;
                    doc["errors"].add("Wrong IP [3]");
                }

                if(!address_staticGateway["0"].is<unsigned int>() || address_staticGateway["0"].as<int>() > 255){
                    error = true;
                    doc["errors"].add("Wrong Gateway [0]");
                }
                if(!address_staticGateway["1"].is<unsigned int>() || address_staticGateway["1"].as<int>() > 255){
                    error = true;
                    doc["errors"].add("Wrong Gateway [1]");
                }
                if(!address_staticGateway["2"].is<unsigned int>() || address_staticGateway["2"].as<int>() > 255 ){
                    error = true;
                    doc["errors"].add("Wrong Gateway [2]");
                }
                if(!address_staticGateway["3"].is<unsigned int>() || address_staticGateway["3"].as<int>() > 255){
                    error = true;
                    doc["errors"].add("Wrong Gateway [3]");
                }
                
                
                if(!address_staticSubnet["0"].is<unsigned int>() || address_staticSubnet["0"].as<int>() > 255 ){
                    error = true;
                    doc["errors"].add("Wrong subnet [0]");
                }
                if(!address_staticSubnet["1"].is<unsigned int>() || address_staticSubnet["1"].as<int>() > 255 ){
                    error = true;
                    doc["errors"].add("Wrong subnet [1]");
                }
                if(!address_staticSubnet["2"].is<unsigned int>() || address_staticSubnet["2"].as<int>() > 255 ){
                    error = true;
                    doc["errors"].add("Wrong subnet [2]");
                }
                if(!address_staticSubnet["3"].is<unsigned int>() || address_staticSubnet["3"].as<int>() > 255 ){
                    error = true;
                    doc["errors"].add("Wrong subnet [3]");
                }
            }


            if(!error){
            Global.config.language.locale = json["locale"].as<String>();
            Global.config.wifi.reconnection.intervall = json["wifi"]["reconTime"];
            Global.config.wifi.ip.enable = address["enStaticIP"];
                if(Global.config.wifi.ip.enable){
                    Global.config.wifi.ip.ip[0] = address_staticIp["0"].as<unsigned int>();
                    Global.config.wifi.ip.ip[1] = address_staticIp["1"].as<unsigned int>();
                    Global.config.wifi.ip.ip[2] = address_staticIp["2"].as<unsigned int>();
                    Global.config.wifi.ip.ip[3] = address_staticIp["3"].as<unsigned int>();

                    Global.config.wifi.ip.gw[0] = address_staticGateway["0"].as<unsigned int>();
                    Global.config.wifi.ip.gw[1] = address_staticGateway["1"].as<unsigned int>();
                    Global.config.wifi.ip.gw[2] = address_staticGateway["2"].as<unsigned int>();
                    Global.config.wifi.ip.gw[3] = address_staticGateway["3"].as<unsigned int>();
                    
                    Global.config.wifi.ip.sub[0] = address_staticSubnet["0"].as<unsigned int>();
                    Global.config.wifi.ip.sub[1] = address_staticSubnet["1"].as<unsigned int>();
                    Global.config.wifi.ip.sub[2] = address_staticSubnet["2"].as<unsigned int>();
                    Global.config.wifi.ip.sub[3] = address_staticSubnet["3"].as<unsigned int>();
                }
            Global.config.save.execute = true;
            }

            if(error){
                response->setCode(500);
            }
            response->setLength();
            request->send(response);
        });

    server.addHandler(boardConfigHandler);

    server.serveStatic("/board/boarcfg.txt", LittleFS, "/cfg/boarcfg.txt");


    server.on("/api/board/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = "See you next time...";
        
        response->setLength();
        request->send(response);
        Global.config.reboot.rebootRequest =true;
    });

    server.on("/api/board/wifi-reset", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();

        doc["execute"] = "See you next time...";
        
        response->setLength();
        request->send(response);

        WiFi.mode(WIFI_AP_STA); // cannot erase if not in STA mode !
        WiFi.persistent(true);
        WiFi.disconnect(true, true);
        WiFi.persistent(false);
        Global.config.reboot.rebootRequest =true;
    });




}

#endif