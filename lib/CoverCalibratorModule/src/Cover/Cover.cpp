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
    if(status == Closed || status == Error){
        return true;
    }
    return false;
}

/* return true if shutter is open, otherwise false */
bool Cover::isOpen(){
    if(status == Opened){
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
    if(status == Opened || status == Error){ /*TODO*/
        return true;
    }
    return false;
}


/* return true if shutter is close, otherwise false */
bool Cover::isClosed(){
    if(status == Closed){
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

    status = Unknown;

}
/* FOR THE FUTURE? */
/*
void Cover::updateLastCommunication() {
    autoClose.lastCommunication = millis();
}*/

Cover::Status Cover::getStatus() const {
    return status;
}

Cover::ActualCommand Cover::getActualCommand() const {
    return actualCmd;
}


/*
Configuration Area
*/


void Cover::getConfiguration(JsonObject obj){
    obj["enable"] = moduleEnable;

    JsonObject servoData = obj["servo"].to<JsonObject>();
    cover.getConfiguration(servoData);

}

void Cover::validateConfiguration(const JsonObject &obj, JsonObject response){

    JsonArray err = response["errors"].to<JsonArray>();
    int retVal = 0;

    Serial.println("---Cover VALIDATION---");


    if(!obj["enable"].is<bool>()){
        err.add("EnableMissing");
        return;
    }

    if(!obj["enable"].as<bool>()){
        return;
    }

    if(!obj["outServo"].is<JsonObject>()){
        err.add("ServoMissing");
        return;
    }


    JsonObject coverCfg = obj["outServo"];
    retVal = cover.validateJsonCfg(coverCfg);

    if(retVal != 1){
        JsonObject e = err.add<JsonObject>();
        e["id"] = 1;
        e["error"] = retVal;
        return;
    }

    /* check if board need a reboot */

    if(coverCfg["pin"].as<unsigned int>() != cover.getPinNumber()){
        Serial.print("reboot");
        response["reboot"] = true;
    }    

}

void Cover::storeConfiguration(JsonObject coverObject, const char* schema){

    tmpCfg.clear();
    

    bool incomingEnable = coverObject["enable"].as<bool>();
    /* copy the data*/
    tmpCfg["enable"] = incomingEnable;

    if(incomingEnable){
        JsonObject servo = tmpCfg["outServo"].to<JsonObject>();
        cover.copyJsonCfg(coverObject["outServo"],servo);
        cover.setMax(servo["maxDeg"]);
        cover.openDeg = servo["openDeg"].as<unsigned int>();
        cover.closeDeg = servo["closeDeg"].as<unsigned int>();
        cover.movingTime = servo["movTime"].as<unsigned int>();
    }


    Preferences pref;
    pref.begin(schema);

    String json;

    serializeJson(tmpCfg,json);

    pref.putString("cover",json);
    pref.end();
    tmpCfg.clear();

}