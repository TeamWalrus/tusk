#pragma once

#include <Arduino.h>

struct WiFiSettings {
  String ssid;
  String password;
  uint8_t channel;
  bool hidden;
};

namespace config_store {

void begin();

WiFiSettings getWiFi();
void setWiFi(const WiFiSettings &settings);

bool getCapturing();
void setCapturing(bool capturing);

bool getDebugMode();
void setDebugMode(bool debug);

}  // namespace config_store
