#pragma once
#define STRINGIFY(x) #x

// Sensor settings
#define SENSOR_ID 1
#define SENSOR_ID_STRING STRINGIFY(SENSOR_ID)
#define SAMPLE_RATE 35

// WiFi settings
#define SSID "Name"
#define PASSWORD "12345678"
#define RETRY_MAX_WIFI 3

// Network transport
#define MQTT false
#define UDP true
#define STATIC_ADDRESS false

// Server UDP settings
#if STATIC_ADDRESS
#define SERVER_IP "192.168.1.1"
#endif
#define SERVER_PORT 5555
#define LOCAL_PORT 1025

// Server MQTT settings
#define BROKER_URL "192.168.1.1"
#define MQTT_PORT 1883
#define MQTT_TOPIC_URI "motion-capture/sensor-" SENSOR_ID_STRING

// MPU configuration pin
#define MPU_SDA_PIN 6
#define MPU_SCL_PIN 7
#define MPU_INT_PIN 10

// Sleep configuration
#define SLEEP_TIME 60 * 1000000 // 60s in microseconds
