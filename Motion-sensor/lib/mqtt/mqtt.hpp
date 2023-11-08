#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

class Mqtt
{
public:
    Mqtt(const char *broker, int port);
    void connect(const char *clientId);
    void publish(const char *topic, const char *message);
    void loop();

private:
    WiFiClient espClient;
    PubSubClient mqttClient;
};
