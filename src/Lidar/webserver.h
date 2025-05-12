#ifndef LIDAR_SERVER
#define LIDAR_SERVER


void lidarWebServer(){

    server.on("/api/lidar-getconfig",               HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->print("{\"lidar\":{ \"startangle\":");
        response->print(Config.lidar.startAngle);
        response->print(",\"endangle\":");
        response->print(Config.lidar.endAngle);
        response->print(",\"mindist\":");
        response->print(Config.lidar.minDist);
        response->print("}}");

        request->send(response);
    });

    AsyncCallbackJsonWebHandler *lidarcfg = new AsyncCallbackJsonWebHandler("/api/lidar-saveconfig", [](AsyncWebServerRequest * request, JsonVariant & json) {
        JsonDocument doc;
        doc = json.as<JsonObject>();        
        Config.lidar.startAngle = doc["startangle"];
        Config.lidar.endAngle = doc["endangle"];
        Config.lidar.minDist = doc["mindist"];
        Config.save.lidar.execute = true;
        request->send(200, "application/json", "{\"accept\": \"ok\"}");
    });


    server.on("/api/lidar",               HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->printf("{\"lidar\":{ \"speed\":");
        response->print(Lidar.speed);
        response->print(",\"anglestep\":");
        response->print(Lidar.angleStep);
        response->print(",\"pointsinscan\":");
        response->print(Lidar.ptInScan);
        response->print(",\"safe\":");
        response->print(Lidar.safe);
        response->print(",\"points\": [ ");
        for (uint16_t i = 0; i < Lidar.ptInScan; i++) {
            if (i != 0) {response->print(",");}
            response->print("{ \"x\":");
            response->print(ld06.getPoints(i)->x);
            response->print(",\"y\":");
            response->print(ld06.getPoints(i)->y);
            response->print("}");
        }
        response->print("]");
        response->print("}");
        response->print("}");
        request->send(response);
    });


    server.addHandler(lidarcfg);

    server.serveStatic("/lidarconfig.txt", SPIFFS, "/lidarconfig.txt");
}

#endif