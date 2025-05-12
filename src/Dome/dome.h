#ifndef DOME_HAND
#define DOME_HAND

bool debug = true;
unsigned long oldCy;
unsigned long oldMillis;
unsigned long ShMoveTimeOut;
unsigned long ShMoveTimeOutAck;
#ifdef ONSTEP_MOUNT
WiFiClient localClient;
#endif

void initDomeConfig(){
    JsonDocument doc;
    File file = SPIFFS.open("/domeconfig.txt", FILE_READ);
    if (!file) {
        Serial.println("Reading Dome config error");
        return;
    }
    DeserializationError error = deserializeJson(doc, file);
    if(error){
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }
    Config.dome.pinStart = doc["pinstart"];
    Config.dome.pinHalt = doc["pinhalt"];
    Config.dome.pinOpen = doc["pinopen"];
    Config.dome.pinClose = doc["pinclose"];
    Config.dome.pinManual = doc["pinmanual"];
    Config.dome.movingTimeOut = doc["tout"];
    Config.dome.enAutoClose = doc["enautoclose"];
    Config.dome.autoCloseTimeOut = doc["autoclose"];
    Config.dome.mountIP = doc["mountip"].as<String>();
    Config.dome.mountPort = doc["mountport"];
    Config.dome.secCheck = doc["seccheck"];
    Config.dome.secStrict = doc["secstrict"];
    file.close();
    Config.read.dome.isValid = true;

    pinMode(Config.dome.pinStart, OUTPUT);
    pinMode(Config.dome.pinHalt, OUTPUT);
    pinMode(Config.dome.pinOpen, INPUT_PULLDOWN);
    pinMode(Config.dome.pinClose, INPUT_PULLDOWN);
    pinMode(Config.dome.pinManual, INPUT_PULLDOWN);
}

void saveDomeConfig(){
    String datasetup;
    JsonDocument doc;
    doc["pinstart"] = Config.dome.pinStart;
    doc["pinhalt"] = Config.dome.pinHalt;
    doc["pinopen"] = Config.dome.pinOpen;
    doc["pinclose"] = Config.dome.pinClose;
    doc["pinmanual"] = Config.dome.pinManual;
    doc["tout"] = Config.dome.movingTimeOut;
    doc["enautoclose"] = Config.dome.enAutoClose;
    doc["autoclose"] = Config.dome.autoCloseTimeOut;
    doc["mountip"] = Config.dome.mountIP;
    doc["mountport"] = Config.dome.mountPort;
    doc["seccheck"] = Config.dome.secCheck;
    doc["secstrict"] = Config.dome.secStrict;
    serializeJson(doc, datasetup);
    File file = SPIFFS.open("/domeconfig.txt", FILE_WRITE);
    file.print(datasetup);
    file.close();
}


void domeInputState() {

    // I used enum for input state for making the cycle code more clean
    if (digitalRead(Config.dome.pinClose) == HIGH && digitalRead(Config.dome.pinOpen) == LOW) {
      Dome.ShutterInputState = ShOnlyClose;
    }
    if ( digitalRead(Config.dome.pinClose) == LOW && digitalRead(Config.dome.pinOpen) == HIGH) {
      Dome.ShutterInputState = ShOnlyOpen;
    }
    if ( digitalRead(Config.dome.pinOpen) == HIGH && digitalRead(Config.dome.pinClose) == HIGH) {
      Dome.ShutterInputState = ShInAll;
    }
    if ( digitalRead(Config.dome.pinOpen) == LOW && digitalRead(Config.dome.pinClose) == LOW) {
      Dome.ShutterInputState = ShInNoOne;
    }
    if ( digitalRead(Config.dome.pinManual) == HIGH) {
      Dome.ShutterManualInputState = true;
    } else {
      Dome.ShutterManualInputState = false;
    }
}

void LastDomeCommandExe() {
  if (Dome.ShutterCommand != Idle) {
    Dome.LastDomeCommand = Dome.ShutterCommand;
  }
}


void domeAutoClose() {

  if (Dome.ShutterInputState == ShOnlyOpen && Config.dome.enAutoClose){

    if ((millis() - (Dome.lastCommunicationMillis)) > (Config.dome.autoCloseTimeOut * 1000 * 60)) {
          Dome.ShutterCommand = CmdClose;
        }

  }
}

bool domeSecurityCheck() {
  // check if it OK to operate the dome:
  // maybe create a FORCE option to bypass checks
  // checks can be: is the mount Parked, is the inclination sensor in the right angle
  // these should be network calls
  bool safe = true;

  Serial.print("dome secCheck: ");
  Serial.println(Config.dome.secCheck);
  if (Config.dome.secCheck) {
    #ifdef ONSTEP_MOUNT
    // check if the mount is parked
    if (localClient.connect(Config.dome.mountIP.c_str(), Config.dome.mountPort)) {                 // Establish a connection

      if (localClient.connected()) {
        localClient.print(":GW#");                      // send data

        while (!localClient.available());                // wait for response
        
        String str = localClient.readStringUntil('#');  // read entire response
        if (str.indexOf('P') != -1) {
          Serial.println("Mount is Parked!");
          if (Config.dome.secStrict) {
            safe = safe && true;
          } else {
            safe = safe || true;
          }
        } else {
          Serial.println("Mount is unsafe!!!");
          if (Config.dome.secStrict) {
            safe = safe && false;
          } else {
            safe = safe || false;
          }
        }
      }
    }
    #endif
    #ifdef LIDAR
    // check the lidar
    if (Lidar.safe) {
      Serial.println("Lidar says the coast is clear!");
      if (Config.dome.secStrict) {
        safe = safe && true;
      } else {
        safe = safe || true;
      }
    } else {
      Serial.println("Lidar says obstacle in the way!!!");
      if (Config.dome.secStrict) {
        safe = safe && false;
      } else {
        safe = safe || false;
      }
    }
    #endif
  }
  Dome.Safe = safe;
  return safe;
}

void domehandlerloop() {
  ShMoveTimeOut = Config.dome.movingTimeOut *1000;
  domeInputState();
  LastDomeCommandExe();
  // TIMEOUT MOVIMENTAZIONE
  if (Dome.Cycle >= 11 && Dome.Cycle <= 12) {
    if ((millis() - ShMoveTimeOutAck) > ShMoveTimeOut) { //input error I wait 10 sec. before done command
      Dome.Cycle = 100;  //Timeout, HALT
    }
  }


  switch (Dome.Cycle)
  {
    case 0:
            Dome.MoveRetry = false;
            if (Dome.ShutterInputState == ShOnlyClose) {
              Dome.ShutterState = ShClose;
            } else if (Dome.ShutterInputState == ShOnlyOpen) {
              Dome.ShutterState = ShOpen;
            } else {
              Dome.ShutterState = ShError;
            }

            if (Dome.ShutterManualInputState) { // the dome button has been pressed
              if (Dome.ShutterInputState == ShOnlyOpen) {
                Dome.ShutterCommand == CmdClose;
              } else {
                Dome.ShutterCommand == CmdOpen;
              }
            }

            if (Dome.ShutterCommand == CmdOpen) {
              if (Dome.ShutterInputState != ShOnlyOpen) {
                Serial.println(Dome.Cycle);
                Dome.ShutterState = ShOpening;
                Dome.Cycle = 10;
              } else {
                Dome.ShutterCommand = Idle;
              }
            }

            if (Dome.ShutterCommand == CmdClose) {
              if (Dome.ShutterInputState != ShOnlyClose) {
                Serial.println(Dome.Cycle);
                Dome.Cycle = 10;
                Dome.ShutterState = ShClosing;
              } else {
                Dome.ShutterCommand = Idle;
                Dome.ShutterState = ShClose;
              }
            }
            if (Dome.ShutterCommand == CmdHalt) {
              Dome.Cycle = 100;
            }
            break;

    /* NO OPENING COMMAND IF ROOF IS OPEN SHOULD ARRIVE, AND VICE VERSA FOR CLOSING COMMAND, BUT ARE ACCEPTED IF NO/EACH TWO INPUT IS IN (safety first) */
    case 10:
            //Open and close cycle are identical, I just hope to reach the right
            //Pulse to start to the motor, ack millis for time out and
            if (!domeSecurityCheck()) {
              Dome.MoveRetry = false;
              Dome.ShutterCommand = Idle;
              Dome.Cycle = 0;
              break;
            }
            ShMoveTimeOutAck = millis();
            #ifdef GATE_BOARD
              digitalWrite(Config.dome.pinStart, HIGH);
            #else
              if (Dome.ShutterCommand == CmdOpen) {
                digitalWrite(Config.dome.pinStart, HIGH);
              }
            
              if (Dome.ShutterCommand == CmdClose) {
                digitalWrite(Config.dome.pinHalt, HIGH);
              }


            #endif
            Dome.Cycle++;

            break;

    case 11:  //Take signal end to loose signal
            if ((millis() - ShMoveTimeOutAck) > 1000) { //Wait 1second anyway
              #ifndef ALEKO
              if (Dome.ShutterInputState == ShInAll || Dome.ShutterInputState == ShInNoOne) {
              #endif
              #ifdef GATE_BOARD
                digitalWrite(Config.dome.pinStart, LOW);
              #endif
              ShMoveTimeOutAck = millis();
              Dome.Cycle++;
              #ifndef ALEKO
              }
              #endif
            }
            break;

    case 12:  //Sensor Reached
            // INIZIO CHECK APERTUA
            if (Dome.ShutterCommand == CmdOpen) {
              if (Dome.ShutterInputState == ShOnlyOpen) { //As expexted direction!
                #ifndef GATE_BOARD
                  digitalWrite(Config.dome.pinStart, LOW);
                #endif
                Dome.ShutterState = ShOpen;
                Dome.Cycle++;
                break;
              }
              if (Dome.ShutterInputState == ShOnlyClose) { //OMG wrong direction!
                if (Dome.MoveRetry == false) {
                  Dome.MoveRetry = true; // just one retry
                  Dome.Cycle = 20;
                } else {
                  Dome.Cycle = 100;  //no ping pong all day, HALT
                }
              } 

            }
            // FINE CHECK APERTUA
            //INIZIO CHECK CHIUSURA
            if (Dome.ShutterCommand == CmdClose) { //OMG wrong direction!
              if (Dome.ShutterInputState == ShOnlyOpen) {
                if (Dome.MoveRetry == false) {
                  Dome.MoveRetry = true; // just one retry
                  Dome.Cycle = 20;
                } else {
                  Dome.Cycle = 100;  //no ping pong all day, HALT
                }
              }
              if (Dome.ShutterInputState == ShOnlyClose) { //As expected direction!
                #ifndef GATE_BOARD
                  digitalWrite(Config.dome.pinHalt, LOW);
                #endif
                Dome.ShutterState = ShClose;
                Dome.Cycle++;
              }
            }
            // FINE CHECK CHIUSURA

            break;

    case 13:
            Dome.MoveRetry = false;
            Dome.ShutterCommand = Idle;
            Dome.Cycle = 0;
            break;


//PING PONG - HALT ASPETTO E RIBADISCO LO START
    case 20:
            
            ShMoveTimeOutAck = millis();
            #ifdef GATE_BOARD
              digitalWrite(Config.dome.pinHalt, HIGH);   //Halt
              digitalWrite(Config.dome.pinStart, LOW);
            #else
              digitalWrite(Config.dome.pinHalt, LOW);   //Shut Down all 
              digitalWrite(Config.dome.pinStart, LOW);
            #endif
            Dome.Cycle++;
            break;

    case 21:
            if ((millis() - ShMoveTimeOutAck) > 1000) { //Wait a second
              digitalWrite(Config.dome.pinHalt, LOW);   
              digitalWrite(Config.dome.pinStart, LOW);
              Dome.Cycle++;
              ShMoveTimeOutAck = millis();
            }      
            break;

    case 22:
            if ((millis() - ShMoveTimeOutAck) > 5000) { //Wait 5 seconds and restart movement
              Dome.Cycle = 10;
            }        
            break;


    /* HALT CYCLE */
    case 100: //halt command for 1sec
            ShMoveTimeOutAck = millis();
            Dome.ShutterState = ShError;

            #ifdef GATE_BOARD
              digitalWrite(Config.dome.pinHalt, HIGH);
              digitalWrite(Config.dome.pinStart, LOW);
            #else
                digitalWrite(Config.dome.pinStart, LOW);
                digitalWrite(Config.dome.pinHalt, LOW);
            #endif
            Dome.Cycle++;
            break;

    case 101: //halt command for 1sec
            if ((millis() - ShMoveTimeOutAck) > 1000) { //Setting Output for 1sec
              digitalWrite(Config.dome.pinHalt, LOW);   
              digitalWrite(Config.dome.pinStart, LOW);
              Dome.Cycle++;
            }
            break;

    case 102: 
    Serial.println("RESET");
            Dome.ShutterCommand = Idle;
            Dome.Cycle = 0;
            Dome.MoveRetry = false;
            break;


    default:
            break;
  }

  domeAutoClose();
}


#include "webserver.h"
#include "alpacaDevices.h"
#include "alpacaManage.h"

void domeServer(){
  DomeAlpacaDevices();
  DomeAlpacaManage();
  domeWebServer();
}

#endif
