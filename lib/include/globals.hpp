#pragma once

// WiFi settings
#define SSID "partage"       // eir98829489
#define PASSWORD "Ub@33?09+" // ejfCu3cpgS

// Server settings
#define SERVER_IP "192.168.188.14"
#define SERVER_PORT 5555
#define LOCAL_PORT 1025

// Sensor settings
#define SENSOR_ID 9
#define SAMPLE_RATE 25

#define BROKER_URL "192.168.188.25"
#define MQTT_PORT 1883
#define TOPIC_DATA "bat_horse/data-" + SENSOR_ID
#define CLIENT_ID "sensor-client_" + SENSOR_ID

// MPU interrupt pin
#define SENSOR_INT_PIN 10

#define PRECISION_DETECTION 100

#define TIMEOUT_DETECTION 0 * 1000
#define SLEEP_TIME 60 * 1000000 // 2s in microseconds

#define MQTT true
#define UDP true
