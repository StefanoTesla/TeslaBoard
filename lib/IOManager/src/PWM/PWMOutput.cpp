#include "PWMOutput.h"
#include <Arduino.h>

#define LOG_TAG "IOPWM"
#define LOGV(...) ESP_LOGV(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)

bool PWMOutput::jsonSetup(JsonObjectConst obj, bool HS) {
  LOGV("Servo channel json setup");
  if (obj["type"].as<int>() != 3) {
    return false;
  }
  setupCommonJson(obj);
  
  channel = -1;

  uint32_t freq;
  if (HS) {
    channel = chMgr->getFastChannel(true);
    freq = 19500;
    LOGD("20kHz PWM channel assigned at channel %d", channel);
  } else {
    freq = 5000;
    channel = chMgr->getFastChannel();
    LOGD("5kHz PWM channel assigned at position %d", channel);
  }

  if (channel < 0) {
    LOGE("Unable to retrive a PWM channel");
    return false;
  }

  // setName(setup["name"]);

  pin = obj["pin"].as<unsigned int>();

  min = 0;
  max = 4095;
  ledcSetup(channel, freq, 11);
  ledcAttachPin(pin, channel);
  LOGD("%s PWM channel configured at pin %d, channel: %d, frequency: %d, ",
       Name, pin, channel, freq);
  return true;
}

void PWMOutput::getConfiguration(JsonObject cfg) {
  cfg["type"] = 3;
  getCommonConfiguration(cfg);
  cfg["pin"] = pin;
}

bool PWMOutput::pinUnusable(int pin) {
  if (pin == 1 or pin == 3 or (pin >= 6 and pin <= 11) or pin == 20 or
      pin == 24 or (pin >= 28 and pin <= 31)) {
    return true;
  }

  if (pin > 33) {
    return true;
  }
  return false;
}

int PWMOutput::validateJsonCfg(JsonObject json) {

  if (!json["pin"].is<unsigned int>()) {
    return -1;
  } else {
    if (pinUnusable(json["pin"].as<unsigned int>())) {
      return -10;
    }
  }

  return 1;
}

void PWMOutput::copyJsonCfg(JsonObject src, JsonObject dest) {
  dest["type"] = 3;
  copyCommonJsonCfg(src,dest);
  dest["pin"] = src["pin"];
}

int PWMOutput::write(int _value) {
  LOGV("PWM command at: %d", _value);
  if (_value >= min && _value <= max) {
    ledcWrite(channel, _value);
    return 1;
  }

  return 0;
}

unsigned int PWMOutput::getMax() { return max; }

int PWMOutput::readPin() {
  /* esp hardware return 4096 when I wrote 4095 bah.. */
  currentDuty = ledcRead(channel);

  if (currentDuty > max) {
    return max;
  }
  return currentDuty;
}

int PWMOutput::status() { return readPin(); }

int PWMOutput::getType() { return 3; }
