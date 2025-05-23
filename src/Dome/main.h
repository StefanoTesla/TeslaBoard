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


//a fast way to set the shutter output instead to repeat every time
void shutterOutput(bool start_open, bool halt_close){
    DomeOutMoveOpen.write(start_open);
    DomeOutHaltClose.write(halt_close);
}

//
//this utility close the shutter if is open for a while and any communication with the board is active
//
void domeAutoClose(){

  if (Dome.Shutter.input == ShInputOnlyOpen){
    if ((Global.actualMillis - (Dome.Shutter.lastCommunicationMillis)) > (Dome.config.data.autoCloseTimeOut * 1000 * 60)) {
          Dome.Shutter.command = ShCommandClose;
        }

  }
}

//
// Open Close shutter Cycle
//
void shutterCycle(){

  if (Dome.Shutter.command == ShCommandIdle) {
    if (Dome.Shutter.input == ShInputOnlyClose) { Dome.Shutter.status = ShStatusClose;}
    if (Dome.Shutter.input == ShInputOnlyOpen) { Dome.Shutter.status = ShStatusOpen;}
    if (Dome.Shutter.input == ShInputNoOne or Dome.Shutter.input == ShInputAll){ Dome.Shutter.status = ShStatusError;}
  }

  if (Dome.Shutter.command == ShCommandHalt and Dome.Shutter.Cycle < 100) {
    Dome.Shutter.Cycle = 100;
  }

  switch (Dome.Shutter.Cycle){
    case 0:
            Dome.Shutter.MoveRetry = false;

          if (Dome.Shutter.command == ShCommandOpen && Dome.Shutter.input != ShInputOnlyOpen) {
            Dome.Shutter.status = ShStatusOpening;
            Dome.Shutter.Cycle = 10;
          } else if(Dome.Shutter.command == ShCommandClose && Dome.Shutter.input != ShInputOnlyClose){
            Dome.Shutter.status = ShStatusClosing;
            Dome.Shutter.Cycle = 10;
          } else {
            Dome.Shutter.command = ShCommandIdle;
          }

      break;

    /* NO OPENING COMMAND IF ROOF IS OPEN SHULD ARRIVE, AND VICE VERSA FOR CLOSING COMMAND, BUT ARE ACCEPTED IF NO/EACH TWO INPUT IS IN (safety first) */
    case 10:
            //Open and close cycle are identical, I just hope to reach the right
            //Pulse to start to the motor, ack millis for time out and
            Dome.Shutter.timeOutAck = Global.actualMillis;
            if(Dome.config.data.driverType == 0){
              shutterOutput(true,false);
            } else if (Dome.config.data.driverType == 1){
              if (Dome.Shutter.command == ShCommandOpen) { shutterOutput(true,false); }
              if (Dome.Shutter.command == ShCommandClose) { shutterOutput(false,true); }
            } else if(Dome.config.data.driverType == 2){
              if (Dome.Shutter.command == ShCommandOpen) { shutterOutput(true,false); }
              if (Dome.Shutter.command == ShCommandClose) { shutterOutput(true,true); }
            } else {
              Serial.println("[DOME]: Wrong driver type configuration!");
              Dome.Shutter.Cycle=100;
            }
            Dome.Shutter.Cycle++;

            break;

    case 11:  //Take signal end to loose signal, for two relay I send the command until the end
            if ((Global.actualMillis - Dome.Shutter.timeOutAck) > 1000) { //Wait 1second anyway
              if (Dome.Shutter.input == ShInputAll || Dome.Shutter.input == ShInputNoOne) {
                if(Dome.config.data.driverType == 0){
                  shutterOutput(false,false);
                }
                Dome.Shutter.Cycle++;
              }
            }
            break;

    case 12:  //Sensor Reached
            // Check Open Cycle
            if (Dome.Shutter.command == ShCommandOpen) {
              if(Global.pulse.second.pulse){
              }
              if (Dome.Shutter.input == ShInputOnlyOpen) { //As aspected direction!
                shutterOutput(false,false);
                Dome.Shutter.status = ShStatusOpen;
                Dome.Shutter.Cycle++;
                break;
              }
              //Open command was sended but I reach the opposite direction
              if(Dome.config.data.driverType == 0){
                if (Dome.Shutter.input == ShInputOnlyClose) { //OMG wrong direction!
                  if (Dome.Shutter.MoveRetry == false) {
                    Dome.Shutter.MoveRetry = true; // just one retry
                    Dome.Shutter.Cycle = 20;
                  } else {
                    Dome.Shutter.Cycle = 100;  //no ping pong all day, HALT
                  }
                }
              }
            }

            // Check Close Cycle
            if (Dome.Shutter.command == ShCommandClose) { 
              if(Global.pulse.second.pulse){
              }
              if (Dome.Shutter.input == ShInputOnlyClose) { //As aspected direction!
                shutterOutput(false,false);
                Dome.Shutter.status = ShStatusClose;
                Dome.Shutter.Cycle++;
              }
              if(Dome.config.data.driverType == 0){
                if (Dome.Shutter.input == ShInputOnlyClose) { //OMG wrong direction!
                  if (Dome.Shutter.MoveRetry == false) {
                    Dome.Shutter.MoveRetry = true; // just one retry
                    Dome.Shutter.Cycle = 20;
                  } else {
                    Dome.Shutter.Cycle = 100;  //no ping pong all day, HALT
                  }
                }
              }
            }
            // FINE CHECK CHIUSURA

            break;

    case 13:
            Dome.Shutter.MoveRetry = false;
            Dome.Shutter.command = ShCommandIdle;
            Dome.Shutter.Cycle = 0;
            break;


//PING PONG - HALT ASPETTO E RIBADISCO LO START
    case 20: 
            Dome.Shutter.timeOutAck = Global.actualMillis;
            shutterOutput(false,true);
            Dome.Shutter.Cycle++;
            break;

    case 21:
            if ((Global.actualMillis - Dome.Shutter.timeOutAck ) > 1000) { //Wait a second
              shutterOutput(false,false); 
              Dome.Shutter.timeOutAck = Global.actualMillis;
              Dome.Shutter.Cycle++;
            }      
            break;

    case 22:
            if ((Global.actualMillis - Dome.Shutter.timeOutAck) > 5000) { //Wait 5 seconds and restart movement
              Dome.Shutter.Cycle = 10;
            }        
            break;


    /* HALT CYCLE */
    case 100: //halt command for 1sec
            Dome.Shutter.timeOutAck = Global.actualMillis;
            Dome.Shutter.status = ShStatusError;
            
            if(Dome.config.data.driverType == 0){
              shutterOutput(false,true);
            } else {
              shutterOutput(false,false);
            }

            Dome.Shutter.Cycle++;
            break;

    case 101: //halt command for 1sec
            if ((Global.actualMillis - Dome.Shutter.timeOutAck) > 1000) { //Setting Output for 1sec
              shutterOutput(false,false);
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


  if (Dome.Shutter.Cycle >= 11 && Dome.Shutter.Cycle <= 12) {
    if ((Global.actualMillis - Dome.Shutter.timeOutAck) > (Dome.config.data.movingTimeOut * 1000)) { //input error I wait 10 sec. before done command
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