#include <Arduino.h>
#include "wifi.hpp"
#include <WiFi.h>
#include "../../include/globals.hpp"

const char *ssid = SSID;         // eir98829489
const char *password = PASSWORD; // ejfCu3cpgS

bool *espGotIp;

void onWiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.printf("Event: SYSTEM_EVENT_STA_GOT_IP\n");
    *espGotIp = true;
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    Serial.printf("Event: SYSTEM_EVENT_STA_CONNECTED\n");
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.printf("Event: SYSTEM_EVENT_STA_DISCONNECTED\n");
    *espGotIp = false;
    break;
  default:
    Serial.printf("Unexpected WiFi event: %d\n", event);
    break;
  }
}

void initWiFi(bool *isGotIP)
{
  Serial.printf("Initializing WiFi\n");

  espGotIp = isGotIP;

  WiFi.onEvent(onWiFiEvent);

  WiFi.begin(ssid, password);

  int index = 0;
  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(10);
    index++;
    if (index == 500) // every 5s
    {
      Serial.printf("Connecting to WiFi...\n");
      index = 0;
    }
  }
}

String getGatewayAddress()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    IPAddress gatewayIP = WiFi.gatewayIP();
    return gatewayIP.toString();
  }
  else
  {
    return "127.0.0.1";
  }
}
