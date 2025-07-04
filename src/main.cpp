#include <Arduino.h>
#include <Wifi.h>
#include "WiFiConfig.h"
#include <SPIFFS.h>

bool waitingWiFiConnect = false;

void setup()
{
  Serial.begin(115200);
  setupWiFiManager();
}

void loop()
{
  handleWiFiManagerLoop();
  if (isConnected)
  {
    Serial.println("Terhubung ke wifi");
  }
  else
  {
    Serial.println("Tidak Terhubung");
  }
  delay(1000);
}
