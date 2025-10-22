#include "Cover.h"
#include "esp_log.h"
#define LOG_TAG "Cover"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

#pragma region Configuration


/* Setup the shutter */
void Cover::begin(JsonDocument doc){

    moduleEnable = doc["enable"].as<bool>();
    openPosition = doc["openPos"].as<unsigned int>();
    closePosition = doc["closePos"].as<unsigned int>();

    if(moduleEnable){
        JsonObject pinServo = doc["outServo"];
        servo.jsonSetup(pinServo);
    }

}



void Cover::getConfiguration(JsonObject obj){
    obj["enable"] = moduleEnable;
    obj["openPos"] = openPosition;
    obj["closePos"] = closePosition;

    JsonObject servoData = obj["outServo"].to<JsonObject>();
    servo.getConfiguration(servoData);

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

    if(!obj["openPos"].is<unsigned int>()){
        err.add("openPosMissing");
        return;
    }
    if(!obj["closePos"].is<unsigned int>()){
        err.add("closePosMissing");
        return;
    }

    unsigned int oD = obj["openPos"].is<unsigned int>();

    if(oD<0 || oD > 100){
        err.add("openPosOutOfRange");
        return;
    }
    unsigned int oC = obj["closePos"].is<unsigned int>();

    if(oD<0 || oD > 100){
        err.add("closePosOutOfRange");
        return;
    }

    if(!obj["outServo"].is<JsonObject>()){
        err.add("ServoMissing");
        return;
    }


    JsonObject coverCfg = obj["outServo"];
    retVal = servo.validateJsonCfg(coverCfg);

    if(retVal != 1){
        JsonObject e = err.add<JsonObject>();
        e["id"] = 2;
        e["error"] = retVal;
        return;
    }

    /* check if board need a reboot */

    if(coverCfg["pin"].as<unsigned int>() != servo.getPinNumber()){
        response["reboot"] = true;
    }    

}

void Cover::storeConfiguration(JsonObject coverObject, const char* schema){

    tmpCfg.clear();
    
    bool incomingEnable = coverObject["enable"].as<bool>();
    /* copy the data*/
    tmpCfg["enable"] = incomingEnable;

    if(incomingEnable){
        tmpCfg["openPos"] = coverObject["openPos"].as<unsigned int>();
        tmpCfg["closePos"] = coverObject["closePos"].as<unsigned int>();


        JsonObject servoObj = tmpCfg["outServo"].to<JsonObject>();
        servo.copyJsonCfg(coverObject["outServo"],servoObj);
        /*set the variables don't need a reboot */
        openPosition =  tmpCfg["openPos"];
        closePosition = tmpCfg["closePos"];
        servo.setMovingTime(servoObj["moveTime"].as<unsigned int>());
    }


    Preferences pref;
    pref.begin(schema);

    String json;

    serializeJson(tmpCfg,json);

    pref.putString("cover",json);
    pref.end();
    tmpCfg.clear();

}

#pragma endregion


/* loop cycle, status and cycle update */
void Cover::loop(){
    servo.loop();
    updateStatus();
    
}

/* return true if you can open, otherwise false */
bool Cover::canOpen(){
    if(!servo.isMoving() && status != Opened ){
        return true;
    }
    return false;
}

/* return true if shutter is open, otherwise false */
bool Cover::isOpen(){

    if(servo.status() == openPosition){
        return true;
    }

    return false;
}

/* send an open command*/
void Cover::open() {
    LOGV("Cover open request");
    if(canOpen()){
        LOGV("You can open");
        servo.goTo(openPosition,false);
    }
    
}

/* return true if you can close, otherwise false */
bool Cover::canClose(){
    if(!servo.isMoving() && status != Closed ){
        return true;
    }
    return false;
}

/* return true if shutter is close, otherwise false */
bool Cover::isClosed(){
    if(servo.status() == closePosition){
        return true;
    }

    return false;
}

/* send a close command */
void Cover::close() {

    servo.goTo(closePosition,false);
}


/* send an halt command */
void Cover::halt(){
    servo.halt();
}

/* return true if shutter is moving, otherwise false */
bool Cover::isMoving() {
    return servo.isMoving() ? true : false;    
}

void Cover::updateStatus() {

    status = NotPresent;

    if(moduleEnable){
        status = Error;

        if(servo.isMoving()){
            status = Moving;
        } else {
            if(isClosed()){
                status = Closed;
            } else if (isOpen()){
                status = Opened;
            }
        }
    }
}


Cover::Status Cover::getStatus() const {
    return status;
}




/*
Configuration Area
*/
