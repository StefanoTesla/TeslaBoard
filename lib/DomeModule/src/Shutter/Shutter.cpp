#include "Shutter.h"
#undef LOG_TAG
#define LOG_TAG "Shutter"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

/* loop cycle, status and cycle update */
void Shutter::loop(){

    debug();
    cycle();
    updateStatus();
}

/* return true if you can open, otherwise false */
bool Shutter::canOpen(){
    if(status != Opened && actualCmd == Idle ){
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
        error = None;
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

void Shutter::setTravelTimeOut(unsigned int time){
    travelTOUT = time * 1000;
    LOGV("New travel Time Out: %d", travelTOUT);
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
        error = None;
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
        if (error != None){
        if(OpenSensor.status() && !CloseSensor.status()){
            status = Opened;
        } else if(CloseSensor.status() && !OpenSensor.status()){
            status = Closed;
        }

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
        LOGE("Trivel time out triggered, sending Halt Command");
        if(actualCmd == Open){
            error = TOutOpening;
        } else if (actualCmd == Close){
            error = TOutClosing;
        }
        halt();
    }
}

void Shutter::checkAutoCloseTimeOut(){

    if(isAutoCloseEnable()){
        if(isOpen()){
            if(millis() - autoClose.lastCommunication > autoClose.waitingTime){
                close();
                LOGE("AutoClose timeout, going to close");
            }
        }
    }
}

/*
Main Cicle
*/
void Shutter::cycle(){

    checkTravelTimeOut();
    checkAutoCloseTimeOut();

    if(actualCmd == Halt && actualStep < HaltBegin){
        actualStep = HaltBegin;
    }


    switch (actualStep)
    {

        // intial step...waiting for a new command
    case WaitForACommand:
        //setOutput(Stop);
        retry = false;
        startTravelMillis = millis();

        if(actualCmd == Close){
                ackTimeout = millis();
                setOutput(goToClose);
                actualStep = WaitSensorLoosing;
                LOGI("Going to close...");
        }

        if(actualCmd == Open){
                ackTimeout = millis();
                setOutput(goToOpen);
                actualStep = WaitSensorLoosing;
                LOGI("Going to open...");
                break;
        }

        
        break;

    case WaitSensorLoosing:
        
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
        //LOGE("Wrong drivertype");
        break;
    }
}

void Shutter::setOutputforGateBoard(outputDirection direction){
    LOGV("Setting the outputs");
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

void Shutter::setAutoCloseTimeMin(unsigned int minutes){
    autoClose.waitingTime = minutes * 60000;
}


void Shutter::debug(){
    log.actual.openState = OpenSensor.status();
    if(log.actual.openState != log.previous.openState){
        if(log.actual.openState){
            LOGV("Open sensor reached");
        } else {
            LOGV("Open sensor lost");
        }
        log.previous.openState = log.actual.openState;
    }

    log.actual.closeState = CloseSensor.status();
    if(log.actual.closeState != log.previous.closeState){
        if(log.actual.closeState){
            LOGV("Close sensor reached");
        } else {
            LOGV("Close sensor lost");
        }
        log.previous.closeState = log.actual.closeState;
    }

    log.actual.startOutput = StartOpen.status();
    if(log.actual.startOutput != log.previous.startOutput){
        if(log.actual.startOutput){
            LOGV("Start Output On");
        } else {
            LOGV("Start Output Off");
        }
        log.previous.startOutput = log.actual.startOutput;
    }

    log.actual.haltOutput = HaltClose.status();
    if(log.actual.haltOutput != log.previous.haltOutput){
        if(log.actual.haltOutput){
            LOGV("Halt Output On");
        } else {
            LOGV("Halt Output Off");
        }
        log.previous.haltOutput = log.actual.haltOutput;
    }

    log.actual.cycle = actualStep;
    if(log.actual.cycle != log.previous.cycle){
        LOGV("Actual step: %d",log.actual.cycle);
        switch (log.actual.cycle)
        {
        case 0:
            LOGV("Cmd wait");
            break;
        case 1:
            LOGV("wait sensor loosing");
            break;
        case 2:
            LOGV("CheckToBeArrived,");
            break;
        case 3:
            LOGV("ArrivedToOpenDestination");
            break;
        case 4:
            LOGV("ArrivedToCloseDestination");
            break;
        case 5:
            LOGV("FinalReset");
            break;
        case 6:
            LOGV("PPSendHaltSignal");
            break;
        case 7:
            LOGV("PPResetHaltSignal,");
            break;
        case 8:
            LOGV("PPWaitBeforeSendANewCommand,");
            break;
        case 9:
            LOGV("PPSendTheCommandAgain");
            break;
        case 10:
            LOGV("HaltBegin");
            break;
        case 11:
            LOGV("HaltWait");
            break;
        case 12:
            LOGV("HaltFinalStep");
            break;        
        default:
            LOGV("don't know where I'am");
            break;
        }
        log.previous.cycle = log.actual.cycle;
    }

    log.actual.cmd = actualCmd;
    if(log.actual.cmd != log.previous.cmd){
        LOGV("Actual cmd: %d",log.actual.cmd);
        log.previous.cmd = log.actual.cmd;
    }

    log.actual.status = status;
    if(log.actual.status != log.previous.status){
        LOGV("Actual status: %d",log.actual.status);
        log.previous.status = log.actual.status;
    }

}


/*
Configuration Area
*/

#pragma region Configuration

void Shutter::begin(const JsonDocument& doc){

    LOGI("start loading configuration");
    
    driverType = doc["driverType"];
    setTravelTimeOut(doc["travelTOut"].as<unsigned int>());

    JsonObjectConst  pinOpen = doc["inOpen"];
    OpenSensor.jsonSetup(pinOpen);

    JsonObjectConst pinClose = doc["inClose"];
    CloseSensor.jsonSetup(pinClose);

    JsonObjectConst pinStart = doc["outStart"];
    StartOpen.jsonSetup(pinStart);

    JsonObjectConst pinHalt = doc["outHalt"];
    HaltClose.jsonSetup(pinHalt);

    if(doc["autoClose"].is<JsonObject>()){
        autoClose.enable = doc["autoClose"]["enable"].as<bool>();
        setAutoCloseTimeMin(doc["autoClose"]["time"].as<unsigned int>());
    }

    moduleEnable = true;
}



void Shutter::getConfiguration(JsonObject obj){

    obj["driverType"] = driverType;
    obj["travelTOut"] = getTravelTimeOut();

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
    autClose["time"] = getAutoCloseTimeMin();

}

void Shutter::validateConfiguration(const JsonObject &obj, JsonObject response){

    JsonArray err = response["errors"].to<JsonArray>();
    int retVal = 0;

    LOGV("---SHUTTER VALIDATION---");

    serializeJson(obj,Serial);

    if(!obj["driverType"].is<int>()){
        err.add("DriveTypeMissing");
        return;
    }


    int tmp = obj["driverType"].as<int>();
    if(tmp < 0 || tmp > 3){
        err.add("DriveTypeOutRange");
        return;
    }
    

    if(!obj["travelTOut"].is<int>()){
        err.add("travelTOutMissing");
        return;
    }


    if(!obj["inOpen"].is<JsonObject>()){
        err.add("InOpenMissing");
        return;
    }


    JsonObject inOpen = obj["inOpen"];
    retVal = OpenSensor.validateJsonCfg(inOpen);

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

    if(!autoCloseObj["time"].is<unsigned int>()){
        err.add("autoCloseTimeMissing");
        return;
    }

    /* check if board need a reboot */

    if(inOpen["pin"].as<unsigned int>() != OpenSensor.getPinNumber()){
        response["reboot"] = true;
    }
    if(inClose["pin"].as<unsigned int>() != CloseSensor.getPinNumber()){
        response["reboot"] = true;
    }
    if(outStart["pin"].as<unsigned int>() != StartOpen.getPinNumber()){
        response["reboot"] = true;
    }
    if(outHalt["pin"].as<unsigned int>() != HaltClose.getPinNumber()){
        response["reboot"] = true;
    }


    

}

void Shutter::storeConfiguration(JsonObject shutterObject, const char* schema){

    tmpCfg.clear();

    /* apply data don't require a reboot*/
    driverType = shutterObject["driverType"];

    setTravelTimeOut(shutterObject["travelTOut"].as<unsigned int>());
    autoClose.enable = shutterObject["autoClose"]["enable"].as<bool>();
    setAutoCloseTimeMin(shutterObject["autoClose"]["time"].as<unsigned int>());

    tmpCfg["driverType"] = driverType;
    tmpCfg["travelTOut"] = getTravelTimeOut();
    tmpCfg["autoClose"]["enable"] = autoClose.enable;
    tmpCfg["autoClose"]["time"] = getAutoCloseTimeMin();

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

    serializeJson(shutterObject,Serial);

    String json;

    serializeJson(tmpCfg,json);

    NvsManager::getInstance().putString("shutter",json);

    tmpCfg.clear();
}


#pragma endregion