#pragma once

/* Wifi */
#define WIFI_STA_SSID "ESP_WIFI"
#define WIFI_STA_PASS "12345678"

#define WIFI_AP_CHANNEL 1
#define WIFI_AP_SSID "ESP_WIFI"
#define WIFI_AP_PASS "12345678"

#define IS_BASE false

#if !IS_BASE
#define SENSOR_NAME "head"
#define WIFI_MODE 1 // STA
#else
#define SENSOR_NAME "base"
#define WIFI_MODE 2 // AP
#endif
