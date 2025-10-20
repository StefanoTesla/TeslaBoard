#include "Shutter.h"
#undef LOG_TAG
#define LOG_TAG "Shutter"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

/* Setup the shutter */
void Shutter::begin(JsonDocument doc){

    LOGI("start loading configuration");
    
    driverType = doc["driverType"];
    travelTOUT = doc["movtimeOut"];

    JsonObject pinOpen = doc["inOpen"];
    OpenSensor.jsonSetup(pinOpen);

    JsonObject pinClose = doc["inClose"];
    CloseSensor.jsonSetup(pinClose);

    JsonObject pinStart = doc["outStart"];
    StartOpen.jsonSetup(pinStart);

    JsonObject pinHalt = doc["outHalt"];
    HaltClose.jsonSetup(pinHalt);

    if(doc["autoClose"].is<JsonObject>()){
        autoClose.enable = doc["autoClose"]["enable"].as<bool>();
        autoClose.waitingTime = doc["autoClose"]["minutes"].as<unsigned int>() * 60000;
    }

    moduleEnable = true;
}


/* loop cycle, status and cycle update */
void Shutter::loop(){

    if(actualStep != previousStep){
        LOGV("Actual step changed to: %d", actualStep);
        previousStep = actualStep;
    }
    oSP=false;
    if(millis()-oneSecondPulse>1000){
        oneSecondPulse = millis();
        oSP=true;
        LOGV("actual step: %d",actualStep);
        LOGV("actual command: %d",actualStep);
        LOGV("actual open sensor: %d",OpenSensor.status());
        LOGV("actual close sensor: %d",CloseSensor.status());
        LOGV("actual start/open output: %d",StartOpen.status());
        LOGV("actual halt/close sensor: %d",HaltClose.status());
    }


    cycle();
    updateStatus();
}

/* return true if you can open, otherwise false */
bool Shutter::canOpen(){
    if(status != Opened && actualCmd == Idle){
        return true;
    }
    return false;
}

/* return true if shutter is open, otherwise false */
bool Shutter::isOpen(){
    if(status == Opened){
        return true;
    }

    return false;
}

/* senda an open command*/
void Shutter::open() {
    if(canOpen()){
        actualCmd = Open;
        LOGI("Open command recived");
    }
    
}

/* return true if you can close, otherwise false */
bool Shutter::canClose(){
    if(status != Closed && actualCmd == Idle){
        return true;
    }
    return false;
}


/* return true if shutter is close, otherwise false */
bool Shutter::isClosed(){
    if(status == Closed){
        return true;
    }

    return false;
}

/* send a close command */
void Shutter::close() {
    if(canClose()){
        LOGI("Closing command recived");
        actualCmd = Close;
    }
    
}


/* send an halt command */
void Shutter::halt(){
    LOGE("HALT command requested");
    actualCmd = Halt;
    actualStep = HaltBegin;
}

/* return true if shutter is moving, otherwise false */
bool Shutter::isMoving() {
    if(actualCmd == Open || actualCmd == Close){
        return true;
    }

    return false;
    
}

void Shutter::updateStatus() {

    status = Error;

    if (actualCmd == Idle){
        if(oSP){LOGV("actual command is idle");}
        if(OpenSensor.status() && !CloseSensor.status()){
            status = Opened;
            if(oSP){LOGV("roof is only open");}
        } else if(CloseSensor.status() && !OpenSensor.status()){
            status = Closed;
            if(oSP){LOGV("roof is only close");}
        }
        if(oSP){LOGV("roof is in error");}
    } else {
        if(actualCmd == Open){
            status = Opening;
            if(oSP){LOGV("[STS] roof is opening");}
        } else if (actualCmd == Close){
            status = Closing;
            if(oSP){LOGV("[STS] roof is closing");}
        }
    }
}

void Shutter::updateLastCommunication() {
    autoClose.lastCommunication = millis();
}

Shutter::Status Shutter::getStatus() const {
    return status;
}

Shutter::ActualCommand Shutter::getActualCommand() const {
    return actualCmd;
}

void Shutter::checkTravelTimeOut(){

    if(isMoving() && (millis()- startTravelMillis > travelTOUT)){
        LOGE("Trivel time out triggered, sending Halt Command");
        halt();
    }
}

void Shutter::checkAutoCloseTimeOut(){

    if(isAutoCloseEnable()){
        if(isOpen()){
            autoClose.remaningTime = (autoClose.waitingTime - (millis() - autoClose.lastCommunication) ) / 1000;

            if(millis() - autoClose.lastCommunication > autoClose.waitingTime){
                close();
                LOGE("AutoClose timeout, going to close");
            }
        } else {
            autoClose.remaningTime = autoClose.waitingTime / 60; //convert minutes in seconds
        }
    }
}

/*
Main Cicle
*/
void Shutter::cycle(){

    checkTravelTimeOut();

    if(actualCmd == Halt && actualStep < HaltBegin){
        actualStep = HaltBegin;
    }


    switch (actualStep)
    {

        // intial step...waiting for a new command
    case WaitForACommand:
        retry = false;
        setOutput(Stop);


        if(actualCmd == Close){
            if(canClose()){
                ackTimeout = millis();
                setOutput(goToClose);
                actualStep = WaitSensorLoosing;
                LOGI("Going to close...");
            } else {
                actualCmd = Idle;
                LOGE("Close command rejected by main cicle");
            }
            break;
        }

        if(actualCmd == Open){
            if(canOpen()){
                ackTimeout = millis();
                setOutput(goToOpen);
                actualStep = WaitSensorLoosing;
                LOGI("Going to open...");
            } else {
                actualCmd = Idle;
                LOGE("Open command rejected by main cicle");
            }
            break;
        }

        
        break;

    case WaitSensorLoosing:

        startTravelMillis = millis();
    
        if(driverType == GateController){
              if(millis() - ackTimeout < 1000){
                break;
              }
              LOGI("Resetting the start signal for gate board controllers");
              setOutput(Stop);
        }

        if(actualCmd == Open){
            LOGI("Waiting for the open signal");
            actualStep = ArrivedToOpenDestination;
            break;
        } else if( actualCmd == Close){
            LOGI("Waiting for the close signal");
            actualStep = ArrivedToCloseDestination;
            break;
        }
        break;


    case ArrivedToOpenDestination:

        if(OpenSensor.status()){
            LOGI("Open sensor reached, goint to Final Reset");
            setOutput(Stop);
            actualStep = FinalReset;
            break;
        }

        if(driverType == GateController){
            if(CloseSensor.status() && !OpenSensor.status()){
                LOGE("Close sensor reached, I wanted to open");
                if(!retry){
                    LOGI("Trying again to open");
                    actualStep = PPSendHaltSignal;
                    break;
                } else {
                    LOGE("Already tried to send a second command. Stop any operation");
                    actualStep = HaltBegin;
                    break;
                }
            }

        }

        break;

    case ArrivedToCloseDestination:

        if(CloseSensor.status()){
            LOGI("Close sensor reached, goint to Final Reset");
            setOutput(Stop);
            actualStep = FinalReset;
            break;
        }

        if(driverType == GateController){
            if(OpenSensor.status() && !CloseSensor.status()){
                LOGE("Close sensor reached, I wanted to open");
                if(!retry){
                    LOGI("Trying again to open");
                    actualStep = PPSendHaltSignal;
                    break;
                } else {
                    LOGE("Already tried to send a second command. Stop any operation");
                    actualStep = HaltBegin;
                    break;
                }
            }

        }
        break;

    case FinalReset:
        LOGI("Requested position reached.");
        travelTime = (millis() - startTravelMillis)/1000;
        LOGI("Travel time: %d sec.", travelTime);
        setOutput(Stop);
        actualCmd = Idle;
        actualStep = WaitForACommand;

        break; 


    case PPSendHaltSignal:
        LOGI("PPCycle, sending an Halt command");
        ackTimeout = millis();
        retry = true;
        actualStep = PPResetHaltSignal;
        setOutput(safeStop);
        break;
    
    case PPResetHaltSignal:
        if(millis() - ackTimeout > 1000){
            LOGI("PPCycle, resetting the Halt command, and wait 5sec. for a new command");
            setOutput(Stop);
            actualStep = PPWaitBeforeSendANewCommand;
            ackTimeout = millis();
            break;
        }
        break;

    case PPWaitBeforeSendANewCommand:
        if(millis() - ackTimeout > 5000){
            LOGI("PPCycle, send a new command");
            actualStep = WaitSensorLoosing;
            ackTimeout = millis();
            break;
        }
        break;

    case HaltBegin:
        LOGE("HALT COMMAND");
        actualCmd = Halt;
        ackTimeout = millis();
        setOutput(safeStop);
        actualStep = HaltWait;
        break;

    case HaltWait:
        if(millis() - ackTimeout > 1000){
            setOutput(Stop);
            LOGI("Resetting all the outputs.");
            actualStep = HaltFinalStep;
        }
        break;

    case HaltFinalStep:
        LOGI("Going back to wait a new command");
        actualCmd = Idle;
        actualStep = WaitForACommand;
        break;

    default:
        LOGE("Undefined step called, sending an halt command");
        actualStep = HaltBegin;
        break;
    }

}


/*

Utilities to move the output based on the board we would like to use
setOutput call the right function based on the board configured
*/
void Shutter::setOutput(outputDirection dir){

    switch (driverType)
    {
    case GateController:
        setOutputforGateBoard(dir);
        break;
    case DirectionalOutput:
        setOutputforDirectionalOutput(dir);
        break;
    case StartAndDirectionOutput:
        setOutputforStartAndDirectionalOutput(dir);
        break;
    
    default:
        break;
    }
}

void Shutter::setOutputforGateBoard(outputDirection direction){
    switch (direction)
    {
        case Stop:
            StartOpen.write(0);
            HaltClose.write(0);
            break;
        case goToOpen:
        case goToClose:
            StartOpen.write(1);
            HaltClose.write(0);
            break;
        case safeStop:
            StartOpen.write(0);
            HaltClose.write(1);
            break;
    }
}

void Shutter::setOutputforDirectionalOutput(outputDirection direction){
    switch (direction)
    {
        case goToOpen:
            StartOpen.write(1);
            HaltClose.write(0);
            break;
        case goToClose:
            StartOpen.write(0);
            HaltClose.write(1);
            break;
        case Stop:
        case safeStop:
            StartOpen.write(0);
            HaltClose.write(0);
            break;
    }
}

void Shutter::setOutputforStartAndDirectionalOutput(outputDirection direction){
    switch (direction)
    {
        case goToOpen:
            StartOpen.write(1);
            HaltClose.write(0);
            break;
        case goToClose:
            StartOpen.write(1);
            HaltClose.write(1);
            break;
        case Stop:
        case safeStop:
            StartOpen.write(0);
            HaltClose.write(0);
            break;
    }
}


/*
Configuration Area
*/


void Shutter::getConfiguration(JsonObject obj){

    obj["driverType"] = driverType;
    obj["travelTOut"] = travelTOUT;

    JsonObject inOpen = obj["inOpen"].to<JsonObject>();
    OpenSensor.getConfiguration(inOpen);

    JsonObject inClose = obj["inClose"].to<JsonObject>();
    CloseSensor.getConfiguration(inClose);

    JsonObject outStart = obj["outStart"].to<JsonObject>();
    StartOpen.getConfiguration(outStart);

    JsonObject outHalt = obj["outHalt"].to<JsonObject>();
    HaltClose.getConfiguration(outHalt);

    JsonObject autClose = obj["autoClose"].to<JsonObject>();
    autClose["enable"] = autoClose.enable;
    autClose["time"] = autoClose.waitingTime;

}

void Shutter::validateConfiguration(const JsonObject &obj, JsonObject response){

    JsonArray err = response["errors"].to<JsonArray>();
    int retVal = 0;

    Serial.println("---SHUTTER VALIDATION---");

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

    /* check if board need a reboot */

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


    

}

void Shutter::storeConfiguration(JsonObject shutterObject, const char* schema){

    tmpCfg.clear();

    /* apply data don't require a reboot*/
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
}