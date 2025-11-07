#ifndef WIFI_APMGR_H
#define WIFI_APMGR_H

#include <Arduino.h>
#include "WiFi/TeslaWiFiAp.h"

class TeslaWiFiAP{

  public:
    TeslaWiFiAP() = default;

    enum Status {
        Closed,
        Closing,
        Starting,
        Started
    };
    void startAP();
    void stopAP();
    Status getStatus();
    void loop();
    

  private:
    Status status = Closed;


    enum cmdAPEnum{
      NONE,
      START,
      STOP
    };

    cmdAPEnum cmd = NONE; 


    enum cycleSteps{
      WAIT_FOR_A_COMMAND,
      STARTING_THE_AP,
      WAIT_AP_RUNNING,
      STOPPING_THE_AP,
      WAIT_AP_CLOSED
    };

    cycleSteps cycle;
};
#endif