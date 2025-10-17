#include "Cover.h"

/* Setup the shutter */
void Cover::begin(JsonDocument doc){

    moduleEnable = doc["enable"];

    if(moduleEnable){
        JsonObject pinServo = doc["servo"];
        cover.jsonSetup(pinServo);
    }

}


/* loop cycle, status and cycle update */
void Cover::loop(){
    updateStatus();
}

/* return true if you can open, otherwise false */
bool Cover::canOpen(){
    if(coverStatus == Closed || coverStatus == Error){
        return true;
    }
    return false;
}

/* return true if shutter is open, otherwise false */
bool Cover::isOpen(){
    if(coverStatus == Opened){
        return true;
    }

    return false;
}

/* senda an open command*/
void Cover::open() {
    if(canOpen()){
        actualCmd = Open;
    }
    
}

/* return true if you can close, otherwise false */
bool Cover::canClose(){
    if(coverStatus == Opened || coverStatus == Error){ /*TODO*/
        return true;
    }
    return false;
}


/* return true if shutter is close, otherwise false */
bool Cover::isClosed(){
    if(coverStatus == Closed){
        return true;
    }

    return false;
}

/* send a close command */
void Cover::close() {
    if(canClose()){
        actualCmd = Close;
    }
    
}


/* send an halt command */
void Cover::halt(){
    actualCmd == Halt;
}

/* return true if shutter is moving, otherwise false */
bool Cover::isMoving() {
    if(actualCmd == Open || actualCmd == Close){
        return true;
    }

    return false;
    
}

void Cover::updateStatus() {

    coverStatus = Unknown;

}
/* FOR THE FUTURE? */
/*
void Cover::updateLastCommunication() {
    autoClose.lastCommunication = millis();
}*/

Cover::Status Cover::getStatus() const {
    return coverStatus;
}

Cover::ActualCommand Cover::getActualCommand() const {
    return actualCmd;
}


/*
Configuration Area
*/


void Cover::getConfiguration(JsonObject obj){
/*    obj["driverType"] = driverType;
    obj["travelTOut"] = travelTOUT;

    JsonObject servoData = obj["servo"].to<JsonObject>();
    servo.getConfiguration(inOpen);

    JsonObject inClose = obj["inClose"].to<JsonObject>();
    CloseSensor.getConfiguration(inClose);

    JsonObject outStart = obj["outStart"].to<JsonObject>();
    StartOpen.getConfiguration(outStart);

    JsonObject outHalt = obj["outHalt"].to<JsonObject>();
    HaltClose.getConfiguration(outHalt);

    JsonObject autClose = obj["autoClose"].to<JsonObject>();
    autClose["enable"] = autoClose.enable;
    autClose["time"] = autoClose.waitingTime;
*/
}

void Cover::validateConfiguration(const JsonObject &obj, JsonObject response){

    JsonArray err = response["errors"].to<JsonArray>();
    int retVal = 0;

    Serial.println("---SHUTTER VALIDATION---");
/*
    serializeJson(obj,Serial);

    if(!obj["driverType"].is<int>()){
        err.add("DriveTypeMissing");
        return;
    }
    Serial.println("driver type is ok");

    int tmp = obj["driverType"].as<int>();
    if(tmp < 0 || tmp > 3){
        err.add("DriveTypeOutRange");
        return;
    }
    
    Serial.println("driver type in range");

    if(!obj["travelTOUT"].is<int>()){
        err.add("travelTOUTMissing");
        return;
    }

    Serial.println("travelTOUT ok");

    if(!obj["inOpen"].is<JsonObject>()){
        err.add("InOpenMissing");
        return;
    }

    Serial.println("inOpen Exist");

    JsonObject inOpen = obj["inOpen"];
    retVal = OpenSensor.validateJsonCfg(inOpen);
    Serial.println(retVal);
    if(retVal != 1){
        JsonObject e = err.add<JsonObject>();
        e["id"] = 1;
        e["error"] = retVal;
        return;
    }

    if(!obj["inClose"].is<JsonObject>()){
        err.add("InCloseMissing");
        return;
    }

    JsonObject inClose = obj["inClose"];
    retVal = CloseSensor.validateJsonCfg(inClose);
    if(retVal != 1){
        JsonObject e = err.add<JsonObject>();
        e["id"] = 2;
        e["error"] = retVal;
        return;
    }

    if(!obj["outStart"].is<JsonObject>()){
        err.add("outStartMissing");
        return;
    }

    JsonObject outStart = obj["outStart"];
    retVal = StartOpen.validateJsonCfg(outStart);
    if(retVal != 1){
        JsonObject e = err.add<JsonObject>();
        e["id"] = 3;
        e["error"] = retVal;
        return;
    }


    if(!obj["outHalt"].is<JsonObject>()){
        err.add("outHaltMissing");
        return;
    }

    JsonObject outHalt = obj["outHalt"];
    retVal = HaltClose.validateJsonCfg(outHalt);
    if(retVal != 1){
        JsonObject e = err.add<JsonObject>();
        e["id"] = 4;
        e["error"] = retVal;
        return;
    }

    if(!obj["autoClose"].is<JsonObject>()){
        err.add("autoCloseMissing");
        return;
    }

    JsonObject autoCloseObj = obj["autoClose"];

    if(!autoCloseObj["enable"].is<bool>()){
        err.add("autoCloseEnableMissing");
        return;
    }

    if(!autoCloseObj["autoCloseTime"].is<unsigned int>()){
        err.add("autoCloseTimeMissing");
        return;
    }
*/
    /* check if board need a reboot */
/*
    if(inOpen["pin"].as<unsigned int>() != OpenSensor.getPinNumber()){
        Serial.print("reboot");
        response["reboot"] = true;
    }
    if(inClose["pin"].as<unsigned int>() != CloseSensor.getPinNumber()){
        Serial.print("reboot");
        response["reboot"] = true;
    }
    if(outStart["pin"].as<unsigned int>() != StartOpen.getPinNumber()){
        Serial.print("reboot");
        response["reboot"] = true;
    }
    if(outHalt["pin"].as<unsigned int>() != HaltClose.getPinNumber()){
        Serial.print("reboot");
        response["reboot"] = true;
    }

*/
    

}

void Cover::storeConfiguration(JsonObject shutterObject, const char* schema){

    tmpCfg.clear();

    /* apply data don't require a reboot*/
    /*
    driverType = shutterObject["driverType"];
    travelTOUT = shutterObject["travelTOUT"].as<unsigned long>();

    autoClose.enable = shutterObject["autoClose"]["enable"].as<bool>();
    autoClose.waitingTime = shutterObject["autoClose"]["autoCloseTime"].as<unsigned long>();

    tmpCfg["driverType"] = driverType;
    tmpCfg["travelTOUT"] = travelTOUT;
    tmpCfg["autoClose"]["enable"] = autoClose.enable;
    tmpCfg["autoClose"]["autoCloseTime"] = autoClose.waitingTime;

    JsonObject inOpen = tmpCfg["inOpen"].to<JsonObject>();
    OpenSensor.copyJsonCfg(shutterObject["inOpen"],inOpen);
    OpenSensor.invert = inOpen["invert"].as<bool>();
    OpenSensor.dOn = inOpen["dOn"].as<unsigned long>();
    OpenSensor.dOff = inOpen["dOff"].as<unsigned long>();

    JsonObject inClose = tmpCfg["inClose"].to<JsonObject>();
    CloseSensor.copyJsonCfg(shutterObject["inClose"],inClose);
    CloseSensor.invert = inClose["invert"].as<bool>();
    CloseSensor.dOn = inClose["dOn"].as<unsigned long>();
    CloseSensor.dOff = inClose["dOff"].as<unsigned long>();
    
    JsonObject outStart = tmpCfg["outStart"].to<JsonObject>();
    StartOpen.copyJsonCfg(shutterObject["outStart"],outStart);
    StartOpen.invert = outStart["invert"].as<bool>();

    JsonObject outHalt = tmpCfg["outHalt"].to<JsonObject>();
    HaltClose.copyJsonCfg(shutterObject["outHalt"],outHalt);
    HaltClose.invert = outHalt["invert"].as<bool>();

    Preferences pref;
    pref.begin(schema);

    serializeJson(shutterObject,Serial);

    String json;

    serializeJson(tmpCfg,json);

    Serial.println(json);
    pref.putString("shutter",json);
    pref.end();
    tmpCfg.clear();

    */
}