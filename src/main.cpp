#include <Arduino.h>
#include <Wifi.h>
#include "WiFiConfig.h"

bool waitingWiFiConnect = false;

void setup()
{
  setupWiFiManager();
}

void loop()
{
  handleWiFiManagerLoop();
}
