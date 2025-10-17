#include "Calibrator.h"

/* Setup the shutter */
void Calibrator::begin(JsonDocument doc){

    Serial.println("Shutter begin");
    
    moduleEnable = doc["enable"];

    JsonObject calib = doc["inOpen"];
    calibrator.jsonSetup(calib);

    moduleEnable = true;
}


/* loop cycle, status and cycle update */
void Calibrator::loop(){
    updateStatus();
}



void Calibrator::updateStatus() {

    status = NotPresent;

    if(moduleEnable){
        if(getBrightness()==0){
            status = Off;
        } else {
            status = Ready;
        }
    }

}



Calibrator::Status Calibrator::getStatus() const {
    return status;
}



/*
Configuration Area
*/


void Calibrator::getConfiguration(JsonObject obj){

    obj["enable"] = moduleEnable;

    JsonObject outPWM = obj["outPWM"].to<JsonObject>();
    calibrator.getConfiguration(outPWM);

}

void Calibrator::validateConfiguration(const JsonObject &obj, JsonObject response){

    JsonArray err = response["errors"].to<JsonArray>();
    int retVal = 0;

    if(!obj["outPWM"].is<JsonObject>()){
        err.add("calibrator is missing");
        return;
    }

    JsonObject calibPin = obj["outPWM"];
    retVal = calibrator.validateJsonCfg(calibPin);

    if(retVal != 1){
        JsonObject e = err.add<JsonObject>();
        e["id"] = 1;
        e["error"] = retVal;
        return;
    }



    /* check if board need a reboot */

    if(calibPin["pin"].as<unsigned int>() != calibrator.getPinNumber()){
        response["reboot"] = true;
    }

    

}

void Calibrator::storeConfiguration(JsonObject calibratorObject, const char* schema){

    tmpCfg.clear();

    tmpCfg["enable"] = calibratorObject["enable"];

    JsonObject outPWM = tmpCfg["outPWM"].to<JsonObject>();
    calibrator.copyJsonCfg(calibratorObject["outPWM"],outPWM);


    String json;
    serializeJson(tmpCfg,json);
    
    Preferences pref;
    pref.begin(schema);

    pref.putString("shutter",json);
    pref.end();
    tmpCfg.clear();
}