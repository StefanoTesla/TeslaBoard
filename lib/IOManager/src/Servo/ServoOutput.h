#ifndef SERVO_OUTPUT_H
#define SERVO_OUTPUT_H

#include "IOBase.h"
#include <ArduinoJson.h>
#include <PWMManager.h>

class ServoOutput : public IOBase {

private:
  int value;
  int cycle = 0;
  bool positioning = false;
  bool overridePosition = false;
  int channel = -1;
  unsigned long movingTime = 0;

  PWMManager *chMgr;

  struct MoveToStruct {
    unsigned int destination = 0; // percentuale 0..100 (solo info)
    int32_t destInDuty = 0;       // duty target finale (signed per i calcoli)
    int32_t startDuty = 0;        // duty di partenza (signed)
    unsigned int startPercentage = 0; // percentuale di partenza (debug)
    unsigned long startMillis = 0;    // tempo di inizio movimento
    unsigned long totalTime = 0; // tempo totale per il movimento completo (ms)
  };

  MoveToStruct moveTo;

  void servoHandler();

public:
  int currentAngle;

  ServoOutput(PWMManager *channelManager) : chMgr(channelManager) {}
  bool jsonSetup(JsonObjectConst setup,bool HS = false) override;
  void getConfiguration(JsonObject cfg) override;
  static int validateJsonCfg(JsonObject obj);
  static bool pinUnusable(int pin);
  static void copyJsonCfg(JsonObject obj, JsonObject dest);

  int write(int _angle) override;
  //    bool goToSlowly(int _percentage=0, bool overridePosition = true);
  int status();
  unsigned int getChannel() { return channel; }
  int getType() override;
  int readPosition();
  bool isMoving() { return positioning; };
  void halt();
  void loop() override;
  void setMax(int _value);
  void goTo(int _percentage, bool direct = false,
            bool _oPos = false); // used only for switch, software decide to
                                 // perform a direct or slow moviment
  bool isReferenced();
  void setMovingTime(unsigned int _time);
  unsigned int getMovingTime() { return movingTime / 1000; }

private:
  void convertPosInduty(int position);
  void handleMovement();
  int readPin() override;
};

#endif