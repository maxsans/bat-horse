#pragma once
#define STRINGIFY(x) #x

// Sensor settings
#define SENSOR_ID 9
#define SENSOR_ID_STRING STRINGIFY(SENSOR_ID)
#define SAMPLE_RATE 25

// WiFi settings
#define SSID "Redmi"        // eir98829489 - partage
#define PASSWORD "maxlefou" // ejfCu3cpgS - Ub@33?09+

// Network transport
#define MQTT false
#define UDP true

// Server UDP settings
#define SERVER_IP "192.168.1.200"
#define SERVER_PORT 5555
#define LOCAL_PORT 1025

// Server MQTT settings
#define BROKER_URL "192.168.137.1"
#define MQTT_PORT 1883
#define MQTT_TOPIC_URI "motion-capture/sensor-" SENSOR_ID_STRING

// MPU configuration pin
#define MPU_SDA_PIN 6
#define MPU_SCL_PIN 7
#define MPU_INT_PIN 10

// Movement detection
#define PRECISION_DETECTION 100 // default is 100

// Sleep configuration
#define TIMEOUT_DETECTION 0 * 1000 // if is set to zero, sleep mode is disabled
#define SLEEP_TIME 10 * 1000000    // 60s in microseconds
