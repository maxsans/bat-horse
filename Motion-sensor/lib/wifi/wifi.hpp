#pragma once
#include <Arduino.h>

void initWiFi(bool *isGotIP);
String getGatewayAddress(void);
