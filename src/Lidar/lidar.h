#ifndef LIDAR_HAND
#define LIDAR_HAND

#include "ld06.h"
LD06 ld06(Serial1);
int mindist;

void initLidarConfig(){
    JsonDocument doc;
    File file = SPIFFS.open("/lidarconfig.txt", FILE_READ);
    if (!file) {
        Serial.println("Reading Lidar config error");
        return;
    }
    DeserializationError error = deserializeJson(doc, file);
    if(error){
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }
    Config.lidar.startAngle = doc["startangle"];
    Config.lidar.endAngle = doc["endangle"];
    Config.lidar.minDist = doc["mindist"];
    file.close();
    Config.read.lidar.isValid = true;
    ld06.init();
    Lidar.safe = false;
    ld06.enableFiltering();            // If filtering is enable only data that are in range will be stored in lidar scan
    // ld06.setIntensityThreshold(200);   // Value from 0 to 255, discard data if intensity is lower than threshold. 200 is a standard value specified in the datasheet to remove false positive detection.
    ld06.setDistanceRange(100, Config.lidar.minDist);  // Values are in mm
    ld06.setAngleRange(Config.lidar.startAngle, Config.lidar.endAngle);        // Values are in ° . You can set "setAngleRange(-10, 10); " or "setAngleRange(350, 10);" to get +- 10° range around 0.
}

void saveLidarConfig(){
    String datasetup;
    JsonDocument doc;
    doc["startangle"] = Config.lidar.startAngle;
    doc["endangle"] = Config.lidar.endAngle;
    doc["mindist"] = Config.lidar.minDist;
    ld06.setDistanceRange(100, Config.lidar.minDist);  // Values are in mm
    ld06.setAngleRange(Config.lidar.startAngle, Config.lidar.endAngle);        // Values are in ° . You can set "setAngleRange(-10, 10); " or "setAngleRange(350, 10);" to get +- 10° range around 0.
    serializeJson(doc, datasetup);
    File file = SPIFFS.open("/lidarconfig.txt", FILE_WRITE);
    file.print(datasetup);
    file.close();
}

void lidarLoop() {
    if (ld06.readScan()) {             // Read lidar packets and return true when a new full 360° scan is available
        Lidar.speed = ld06.getSpeed();
        Lidar.angleStep = ld06.getAngleStep();
        Lidar.ptInScan = ld06.getNbPointsInScan();
        mindist = Config.lidar.minDist;
        for (uint16_t i = 0; i < Lidar.ptInScan; i++) {
            if (ld06.getPoints(i)->distance < mindist){
                mindist = ld06.getPoints(i)->distance;
            }
        }
        if (mindist < Config.lidar.minDist) {
            Lidar.safe = false;
        } else {
            Lidar.safe = true;
        }
        // ld06.printScanTeleplot(Serial);  // Print full scan using teleplot format (check :https://teleplot.fr/)
        /*
        // Other displays examples and getters
        Lidar.ld06.printScanCSV(Serial);  // Print scan in csv format
        if (Lidar.ld06.isNewScan()) {     // Even if fullScan is disabled you can know when last data chunk have a loop closure
          Serial.println("This is a new scan! ");
        }
        
        */
    }
}

#include "webserver.h"

void lidarServer(){
    lidarWebServer();
}

#endif