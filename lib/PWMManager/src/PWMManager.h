#ifndef PWMMGR_H
#define PWMMGR_H

#include <Arduino.h>

/*

    This "library" is necesary to keep track of the ledChannel used by the IOManager
    ESP32 got two kind of timers, hardware and software, using the espressif sdk you can decide wich kind of timer you wan't to use
    In the arduino framework the first 8 channels (0-7) are connected to hardware timers and the latest 8 are connected to software timers

    ESP32 S3 have just 8 pwm channels, so in any case we need to use the "fast Timer, even if are slow"

*/
class PWMManager {
public:

    PWMManager() = default;
    
    int getFastChannel(bool strict=false){
        for (int i = 0; i < 8; i++) { if(fastTimer[i]==0){ fastTimer[i]=1; return i; } }

        if(strict){ return -1; }
        #ifndef IS_ESP32S3
        for (int i = 0; i < 8; i++) { if(slowTimer[i]==0){ slowTimer[i]=1; return i+8; } }  
        #endif
        return -1;
    }

    int getSlowChannel(bool strict=false){
        #ifndef IS_ESP32S3
        for (int i = 0; i < 8; i++) { if(slowTimer[i]==0){ slowTimer[i]=1; return i+8; } }

        if(strict){ return -1; }
        #endif
        for (int i = 0; i < 8; i++) { if(fastTimer[i]==0){ fastTimer[i]=1; return i; } }

        return -1;
    }

private:

  int fastTimer[8] {0};
  int slowTimer[8] {0};

};

#endif