#include "Cover.h"
#include "esp_log.h"
#define LOG_TAG "Cover"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)
#define COVERC_SCHEMA_NAME "cccfg"

#pragma region nvsHandler

bool Cover::openNVS(bool readOnly) {

  switch (nvsStatus) {

    // nvs is closed, i need to open according by the readOnly
  case CLOSED:
    LOGV("NVS seems to be closed");
    if (nvs.begin(COVERC_SCHEMA_NAME, readOnly)) {
      if (readOnly) {
        LOGV("NVS opened in readonly");
        nvsStatus = OPEN_READOLNY;
      } else {
        LOGV("NVS opened with write rights");
        nvsStatus = OPEN_WRITE;
      }
      return true;
    } else {
      LOGE("Error opening the NVS");
      nvsStatus = CLOSED;
      return false;
    }
    break;

  case OPEN_READOLNY:
    LOGV("NVS seems open in read only");
    if (!readOnly) {
      closeNVS();
      if (nvs.begin(COVERC_SCHEMA_NAME, false)) {
        nvsStatus = OPEN_WRITE;
        LOGV("NVS opened with write rights");
        return true;
      } else {
        LOGV("Error during opening NVS with write rights");
        return false;
      }
    } else {
      LOGV("NVS already open in read only");
      return true;
    }
    break;

  case OPEN_WRITE:
    LOGV("NVS seems open with write rights");
    if (readOnly) {
      closeNVS();
      nvsStatus = CLOSED;
      if (nvs.begin(COVERC_SCHEMA_NAME, true)) {
        nvsStatus = OPEN_READOLNY;
        LOGV("NVS opened in read only");
        return true;
      } else {
        LOGV("Error during opening NVS in read only");
        return false;
      }
    } else {
      LOGV("NVS already open with write rights");
      return true;
    }
    break;

  default:
    LOGE("Unknown NVS status: %d", nvsStatus);
    return false;
    break;
  }

  LOGE("Arrived at the buttom of the function, don't know what happed..");
  return false;
}

void Cover::closeNVS() {
  if (nvsStatus != CLOSED) {
    nvs.end();
    nvsStatus = CLOSED;
    LOGV("NVS closed");
  } else {
    LOGV("NVS already closed");
  }
}

#pragma endregion

#pragma region Configuration

/* Setup the shutter */
void Cover::begin(const JsonDocument &doc) {

  moduleEnable = doc["enable"].as<bool>();
  openPosition = doc["openPos"].as<unsigned int>();
  closePosition = doc["closePos"].as<unsigned int>();
  storePosition = doc["storePos"].as<bool>();

  if (moduleEnable) {
    JsonObjectConst pinServo = doc["outServo"];
    servo.jsonSetup(pinServo);

    if(storePosition){
      int lastPos = getLastPosition();
      if(lastPos >= 0){
        servo.goTo(lastPos,true);
      }
    }

  }
}

void Cover::getConfiguration(JsonObject obj) {
  obj["enable"] = moduleEnable;
  obj["openPos"] = openPosition;
  obj["closePos"] = closePosition;
  obj["storePos"] = storePosition;

  JsonObject servoData = obj["outServo"].to<JsonObject>();
  servo.getConfiguration(servoData);
}

void Cover::validateConfiguration(const JsonObject &obj, JsonObject response) {

  JsonArray err = response["errors"].to<JsonArray>();
  int retVal = 0;

  LOGV("---Cover VALIDATION---");

  if (!obj["enable"].is<bool>()) {
    err.add("EnableMissing");
    return;
  }

  if (!obj["enable"].as<bool>()) {
    return;
  }

  if (!obj["openPos"].is<unsigned int>()) {
    err.add("openPosMissing");
    return;
  }
  if (!obj["closePos"].is<unsigned int>()) {
    err.add("closePosMissing");
    return;
  }

  unsigned int oD = obj["openPos"].as<unsigned int>();

  if (oD > 100) {
    err.add("openPosOutOfRange");
    return;
  }
  unsigned int oC = obj["closePos"].as<unsigned int>();

  if (oC > 100) {
    err.add("closePosOutOfRange");
    return;
  }

  if (!obj["outServo"].is<JsonObject>()) {
    err.add("ServoMissing");
    return;
  }

  JsonObject coverCfg = obj["outServo"];
  retVal = servo.validateJsonCfg(coverCfg);

  if (retVal != 1) {
    JsonObject e = err.add<JsonObject>();
    e["id"] = 2;
    e["error"] = retVal;
    return;
  }

  /* check if board need a reboot */

  if (
    coverCfg["pin"].as<unsigned int>() != servo.getPinNumber()
    || moduleEnable != obj["enable"].as<bool>()
  ) {
    response["reboot"] = true;
  }
}

void Cover::storeConfiguration(JsonObject coverObject) {

  tmpCfg.clear();

  bool incomingEnable = coverObject["enable"].as<bool>();
  /* copy the data*/
  tmpCfg["enable"] = incomingEnable;

  if (incomingEnable) {
    tmpCfg["openPos"] = coverObject["openPos"].as<unsigned int>();
    tmpCfg["closePos"] = coverObject["closePos"].as<unsigned int>();

    JsonObject servoObj = tmpCfg["outServo"].to<JsonObject>();
    servo.copyJsonCfg(coverObject["outServo"], servoObj);
    /*set the variables don't need a reboot */
    openPosition = tmpCfg["openPos"];
    closePosition = tmpCfg["closePos"];
    servo.setMovingTime(servoObj["moveTime"].as<unsigned int>());
  }

  openNVS(false);
  String json;
  serializeJson(tmpCfg, json);
  nvs.putString("cover", json);
  closeNVS();
  tmpCfg.clear();
}

#pragma endregion

/* loop cycle, status and cycle update */
void Cover::loop() {
  servo.loop();
  updateStatus();

  if(storePosition){ storeLastPosition(); }
}

/* return true if you can open (servo is not moving and is not already open),
 * otherwise false */
bool Cover::canOpen() {
  if (!servo.isMoving() && status != Opened) {
    return true;
  }
  return false;
}

/* return true if shutter is open, otherwise false */
bool Cover::isOpen() {

  if (servo.status() == openPosition) {
    return true;
  }

  return false;
}

/* send an open command*/
void Cover::open() {
  if (canOpen()) {
    servo.goTo(openPosition);
  }
}

/* return true if you can close, otherwise false */
bool Cover::canClose() {
  if (!servo.isMoving() && status != Closed) {
    return true;
  }
  return false;
}

/* return true if shutter is close, otherwise false */
bool Cover::isClosed() {
  if (servo.status() == closePosition) {
    return true;
  }

  return false;
}

/* send a close command */
void Cover::close() {
  if (canClose()) {
    servo.goTo(closePosition, false);
  }
}

/* send an halt command */
void Cover::halt() { servo.halt(); }

/* return true if shutter is moving, otherwise false */
bool Cover::isMoving() { return servo.isMoving() ? true : false; }

void Cover::updateStatus() {

  if (status != oldStatus) {
    LOGV("Cover status: %d", status);
    oldStatus = status;
  }

  if (!moduleEnable) {
    status = NotPresent;
    return;
  }

  if (servo.isMoving()) {
    status = Moving;
    return;
  }

  if (isClosed()) {
    status = Closed;
    return;
  }
  if (isOpen()) {
    status = Opened;
    return;
  }

  status = Unknown;
}

Cover::Status Cover::getStatus() const { return status; }

void Cover::storeLastPosition(){

  if(servo.isMoving()){
    return;
  }

  if(millis() - lastPosMillis < 300000){
    return;
    //wait until timer don't reach 5minutes
  }
  LOGV("Going to store the servo position");



  int servoPos = servo.readPosition();

  if(lastPosition != servoPos
    && servoPos >= 0  ){
    LOGV("Storing the cover position");
    lastPosition = servo.readPosition();
    if(openNVS(false)){
      nvs.putInt("cPos",lastPosition);
    } else {
      LOGE("Unable to store last cover position");
    }

    closeNVS();
    
  }

  lastPosMillis = millis();
}

//return the last know cover position
//if negative value, was never witten
int Cover::getLastPosition(){
  openNVS(true);
  return nvs.getInt("cPos",-1);
}