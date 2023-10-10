#pragma once
#include <Arduino.h>

void initWiFi(bool *isGotIP);
const char *getGatewayAddress();
