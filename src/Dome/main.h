#ifndef DOME_MAIN
#define DOME_MAIN


DigitalInput DomeInOpen;
DigitalInput DomeInClose;
DigitalOutput DomeOutMoveOpen;
DigitalOutput DomeOutHaltClose;

#include "config.h"



//
// Read shutter input status
//
void domeInputRead(){
    int status = 0;

    status = (DomeInClose.status() ? 1 : 0) + (DomeInOpen.status() ? 2 : 0);
    
    switch (status)
    {
    case 0:
      Dome.Shutter.input = ShInputNoOne;
      break;
    case 1:
      Dome.Shutter.input = ShInputOnlyClose;
      break;
    case 2:
      Dome.Shutter.input = ShInputOnlyOpen;
      break;
    case 3:
      Dome.Shutter.input = ShInputAll;
      break;
    
    default:
      break;
    }
}


//
// callback function from shutter output to encrease debug speed
//
void GateContorollerOutputHandler(enumCommandDir direction){
    switch (direction)
    {
      case ShCmdDirHalt:
        Serial.println("[Dome] HALT SIGNLAS");
        DomeOutMoveOpen.write(false);
        DomeOutHaltClose.write(true);
        break;
      case ShCmdDirStop:
      Serial.println("[Dome] OFF SIGNLAS");
        DomeOutMoveOpen.write(false);
        DomeOutHaltClose.write(false);
        break;
      case ShCmdDirOpen:
      case ShCmdDirClose:
      Serial.println("[Dome] START SIGNLAS");
        DomeOutMoveOpen.write(true);
        DomeOutHaltClose.write(false);
        break;

      default:
        break;
    }

}
void DirectionOutputHandler(enumCommandDir direction){
    switch (direction)
    {
      case ShCmdDirHalt:
      case ShCmdDirStop:
        DomeOutMoveOpen.write(false);
        DomeOutHaltClose.write(false);
        break;
      case ShCmdDirOpen:
        DomeOutMoveOpen.write(true);
        DomeOutHaltClose.write(false);
      case ShCmdDirClose:
        DomeOutMoveOpen.write(false);
        DomeOutHaltClose.write(true);
        break;

      default:
        break;
    }
}
void StartAndDirectionOutputHandler(enumCommandDir direction){
    switch (direction)
    {
      case ShCmdDirHalt:
      case ShCmdDirStop:
        DomeOutMoveOpen.write(false);
        DomeOutHaltClose.write(false);
        break;
      case ShCmdDirOpen:
        DomeOutMoveOpen.write(true);
        DomeOutHaltClose.write(false);
      case ShCmdDirClose:
        DomeOutMoveOpen.write(true);
        DomeOutHaltClose.write(true);
        break;

      default:
        break;
    }
}

//a fast way to set the shutter output instead to repeat every time
void shutterOutput(enumCommandDir direction){
    if(Dome.config.data.driverType == dtGateController){
      GateContorollerOutputHandler(direction);
    } else if(Dome.config.data.driverType == dtDirection){
      DirectionOutputHandler(direction);
    } else if(Dome.config.data.driverType == dtStartAndDirection){
      StartAndDirectionOutputHandler(direction);
    }
}

//
//this utility close the shutter if is open for a while and any communication with the board is active
//
void domeAutoClose(){

  if (Dome.Shutter.input == ShInputOnlyOpen){
    if ((millis() - (Dome.Shutter.lastCommunicationMillis)) > (Dome.config.data.autoCloseTimeOut * 1000 * 60)) {
          Dome.Shutter.command = ShCommandClose;
        }
  }
}

//
// Open Close shutter Cycle
//
void shutterCycle(){

  if (Dome.Shutter.command == ShCommandIdle) {
    if (Dome.Shutter.input == ShInputOnlyClose) { 
      Dome.Shutter.status = ShStatusClose;
    } else if (Dome.Shutter.input == ShInputOnlyOpen) {
      Dome.Shutter.status = ShStatusOpen;
    } else { Dome.Shutter.status = ShStatusError;}
  }

  if (Dome.Shutter.command == ShCommandHalt and Dome.Shutter.Cycle < 100) {
    Dome.Shutter.Cycle = 100;

  }

  switch (Dome.Shutter.Cycle){
    case 0:
          Dome.Shutter.MoveRetry = false;

          if (Dome.Shutter.command == ShCommandOpen 
              && Dome.Shutter.input != ShInputOnlyOpen) {
                Dome.Shutter.status = ShStatusOpening;
                Dome.Shutter.timeOutAck = millis();
                Dome.Shutter.Cycle = 10;
          } else if(Dome.Shutter.command == ShCommandClose 
              && Dome.Shutter.input != ShInputOnlyClose){
                Dome.Shutter.status = ShStatusClosing;
                Dome.Shutter.Cycle = 10;
                ;
          } else {
            Dome.Shutter.command = ShCommandIdle;
            shutterOutput(ShCmdDirStop);
          }
      break;

    /* NO OPENING COMMAND IF ROOF IS OPEN SHULD ARRIVE, AND VICE VERSA FOR CLOSING COMMAND, BUT ARE ACCEPTED IF NO/EACH TWO INPUT IS IN (safety first) */
    case 10:
            //Open and close cycle are identical, I just hope to reach the right direction
            //Pulse to start to the motor, ack millis for time out and
            Dome.Shutter.timeOutAck = millis();
            if (Dome.Shutter.command == ShCommandOpen) { 
              shutterOutput(ShCmdDirOpen);
              Dome.Shutter.Cycle++;
              break;
            } else if (Dome.Shutter.command == ShCommandClose) { 
              shutterOutput(ShCmdDirClose);
              Dome.Shutter.Cycle++;
              break;
            } else {
              Serial.println("[DOME][ERR] No command but in cycle? Going to HALT Mode");
              Dome.Shutter.Cycle=100;
            }
            break;

    case 11:  //Waiting for the sensors to go away

            if(Dome.config.data.driverType == dtGateController){
              if(millis()- Dome.Shutter.timeOutAck < 1000){
                //send at least one second pulse for star
                break;
              }
              shutterOutput(ShCmdDirStop);
            }

            //check where I want to go
            if(Dome.Shutter.command == ShCommandOpen){
              Dome.Shutter.Cycle = 15;
            } else if (Dome.Shutter.command == ShCommandClose){
              Dome.Shutter.Cycle = 20;
            }
              
            break;

    case 15:  //Open cycle, check sensor status
            if (Dome.Shutter.input == ShInputOnlyOpen) { //As aspected direction!
              Dome.Shutter.Cycle=25;
              break;
            }

            //this can happend only with gate board
            //Open command was sended but I reach the opposite direction
            if(Dome.config.data.driverType == dtGateController && Dome.Shutter.input == ShInputOnlyClose) { //wrong direction!
              if (Dome.Shutter.MoveRetry == false) {
                Dome.Shutter.Cycle = 30;
              } else {
                Dome.Shutter.Cycle = 100;  //no ping pong all day, HALT
              }
            }
            break;

    case 20: //Close cycle, check sensor status
            if (Dome.Shutter.input == ShInputOnlyClose) { //As aspected direction!
              Dome.Shutter.Cycle=25;
              break;
            }
            //Close command was sended but I reach the opposite direction
            //this can happend only with gate board
            if(Dome.config.data.driverType == dtGateController && Dome.Shutter.input == ShInputOnlyOpen) { //wrong direction!
              if (Dome.Shutter.MoveRetry == false) {
                Dome.Shutter.Cycle = 30;
              } else {
                Dome.Shutter.Cycle = 100;  //no ping pong all day, HALT
              }
            }
            // FINE CHECK CHIUSURA

            break;

    case 25:
            shutterOutput(ShCmdDirStop);
            Dome.Shutter.command = ShCommandIdle;
            Dome.Shutter.Cycle = 0;
            break;


//PING PONG - HALT ASPETTO E RIBADISCO LO START
    case 30: 
            Dome.Shutter.timeOutAck = millis();
            Dome.Shutter.MoveRetry = true; // just one retry
            shutterOutput(ShCmdDirHalt);
            Dome.Shutter.Cycle++;
            break;

    case 31:
            if ((millis() - Dome.Shutter.timeOutAck ) > 1000) { //Wait a second
              shutterOutput(ShCmdDirStop); 
              Dome.Shutter.timeOutAck = millis();
              Dome.Shutter.Cycle++;
            }      
            break;

    case 32:
            if ((millis() - Dome.Shutter.timeOutAck) > 5000) { //Wait 5 seconds and restart movement
              Dome.Shutter.Cycle = 10;
            }        
            break;


    /* HALT CYCLE */
    case 100: //halt command for 1sec
            Serial.println("[Dome] HALT");
            Dome.Shutter.timeOutAck = millis();
            Dome.Shutter.status = ShStatusError;
            shutterOutput(ShCmdDirHalt);
            Dome.Shutter.Cycle++;
            break;

    case 101: //halt command for 1sec
            if ((millis() - Dome.Shutter.timeOutAck) > 1000) { //Setting Output for 1sec
              shutterOutput(ShCmdDirStop);
              Dome.Shutter.Cycle++;
            }
            break;

    case 102:
            Dome.Shutter.command = ShCommandIdle;
            Dome.Shutter.Cycle = 0;
            Dome.Shutter.MoveRetry = false;
            break;

    default:
            Dome.Shutter.Cycle = 100;
            break;
  }

     
}

//
// Store last Shutter Command
//
void lastShutterCommand(){
  if (Dome.Shutter.command != Dome.Shutter.LastDomeCommand && Dome.Shutter.command != ShCommandIdle) {
    Dome.Shutter.LastDomeCommand = Dome.Shutter.command;
  }
}

void domeLoop() {
  domeInputRead();
  if(Dome.config.data.enAutoClose){
    domeAutoClose();
  }
  
  shutterCycle();
  lastShutterCommand();


  if (Dome.Shutter.Cycle >= 11 && Dome.Shutter.Cycle <= 25) {
    if ((millis() - Dome.Shutter.timeOutAck) > (Dome.config.data.movingTimeOut * 1000)) { //input error I wait 10 sec. before done command
      Serial.println("DOME: SHUTTER TIMEOUT");
      Dome.Shutter.command = ShCommandHalt;  //Timeout, HALT
    }
  }

  if (Dome.config.Save.execute){
    saveDomeConfig();
    Dome.config.Save.execute = false;
  }

}


#include "webserver.h"
#include "alpacaDevice.h"
#include "alpacaManage.h"

void domeRequestHandler(){
  domeAlpacaDevices();
  domeAlpacaManage();
  domeWebServer();
}
#endif