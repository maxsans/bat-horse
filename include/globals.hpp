#pragma once

/* Wifi */

#define MQTT_BROKER_URL "mqtt://192.168.137.123"

#define IS_BASE false

#if !IS_BASE
#define WIFI_STA_SSID "ESP_WIFI"
#define WIFI_STA_PASS "12345678"
#define WIFI_STA_CHANNEL 1

#define WIFI_AP_SSID "ESP_WIFI"
#define WIFI_AP_PASS "12345678"
#define WIFI_AP_CHANNEL 2

#define WIFI_PEER_CHANNEL 0

#define SENSOR_NAME "leg"
#define WIFI_MODE 1 // STA
#else
#define WIFI_STA_SSID "Nom"      //  eir98829489
#define WIFI_STA_PASS "12345678" //  ejfCu3cpgS
#define WIFI_STA_CHANNEL 2

#define WIFI_AP_SSID "ESP_WIFI"
#define WIFI_AP_PASS "12345678"
#define WIFI_AP_CHANNEL 1

#define WIFI_PEER_CHANNEL 0

#define SENSOR_NAME "base"
#define WIFI_MODE 3 // APSTA

#define RECORD_TOPIC "bat-horse/record"
#endif
