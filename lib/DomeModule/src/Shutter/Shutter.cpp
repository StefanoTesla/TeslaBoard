#include "Shutter.h"

/* Setup the shutter */
void Shutter::begin(JsonDocument doc){

    Serial.println("Shutter begin");
    
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
    cycle();
    updateStatus();
}

/* return true if you can open, otherwise false */
bool Shutter::canOpen(){
    if(status == Closed || status == Error){
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
    }
    
}

/* return true if you can close, otherwise false */
bool Shutter::canClose(){
    if(status == Opened || status == Error){
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
        actualCmd = Close;
    }
    
}


/* send an halt command */
void Shutter::halt(){
    actualCmd == Halt;
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
        
        if(OpenSensor.status() && !CloseSensor.status()){
            status = Opened;
        } else if(CloseSensor.status() && !OpenSensor.status()){
            status = Closed;
        }
    } else {

        if(actualCmd == Open){
            status = Opening;
        } else if (actualCmd == Close){
            status = Closing;
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
        actualCmd = Halt;
    }

}


void Shutter::checkAutoCloseTimeOut(){

    if(isAutoCloseEnable()){
        if(isOpen()){
            autoClose.remaningTime = (autoClose.waitingTime - (millis() - autoClose.lastCommunication) ) / 1000;

            if(millis() - autoClose.lastCommunication > autoClose.waitingTime){
                close();
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
            } else {
                actualCmd = Idle;
            }
            break;
        }

        if(actualCmd == Open){
            if(canOpen()){
                ackTimeout = millis();
                setOutput(goToOpen);
                actualStep = WaitSensorLoosing;
            } else {
                actualCmd = Idle;
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
              setOutput(Stop);
        }

        if(actualCmd == Open){
            actualStep = ArrivedToOpenDestination;
            break;
        } else if( actualCmd == Close){
            actualStep = ArrivedToCloseDestination;
            break;
        }
        break;


    case ArrivedToOpenDestination:

        if(OpenSensor.status()){
            setOutput(Stop);
            actualStep = FinalReset;
            break;
        }

        if(driverType == GateController){
            if(CloseSensor.status() && !OpenSensor.status()){
                if(!retry){
                    actualStep = PPSendHaltSignal;
                    break;
                } else {
                    actualStep = HaltBegin;
                    break;
                }
            }

        }

        break;

    case ArrivedToCloseDestination:

        if(CloseSensor.status()){
            setOutput(Stop);
            actualStep = FinalReset;
            break;
        }

        if(driverType == GateController){
            if(OpenSensor.status() && !CloseSensor.status()){
                if(!retry){
                    actualStep = PPSendHaltSignal;
                    break;
                } else {
                    actualStep = HaltBegin;
                    break;
                }
            }

        }
        break;

    case FinalReset:
        travelTime = millis() - startTravelMillis;
        setOutput(Stop);
        actualCmd = Idle;
        actualStep = WaitForACommand;
        break; 


    case PPSendHaltSignal:
        ackTimeout = millis();
        retry = true;
        actualStep = PPResetHaltSignal;
        setOutput(safeStop);
        break;
    
    case PPResetHaltSignal:
        if(millis() - ackTimeout > 1000){
            setOutput(Stop);
            actualStep = PPWaitBeforeSendANewCommand;
            ackTimeout = millis();
            break;
        }
        break;

    case PPWaitBeforeSendANewCommand:
        if(millis() - ackTimeout > 5000){
            actualStep = WaitSensorLoosing;
            ackTimeout = millis();
            break;
        }
        break;

    case HaltBegin:
        actualCmd = Halt;
        ackTimeout = millis();
        setOutput(safeStop);
        actualStep = HaltWait;
        break;

    case HaltWait:
        if(millis() - ackTimeout > 1000){
            setOutput(Stop);
            actualStep = HaltFinalStep;
        }
        break;

    case HaltFinalStep:
        actualCmd = Idle;
        actualStep = WaitForACommand;
        break;

    default:
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