#include "mqtt.hpp"

Mqtt::Mqtt(const char *broker, int port)
    : mqttClient(espClient)
{
    mqttClient.setServer(broker, port);
}

void Mqtt::connect(const char *clientId)
{
    while (!mqttClient.connected())
    {
        if (mqttClient.connect(clientId))
        {
            Serial.println("Connected to MQTT broker");
        }
        else
        {
            Serial.print("MQTT connection failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void Mqtt::publish(const char *topic, const char *message)
{
    mqttClient.publish(topic, message);
}

void Mqtt::loop()
{
    mqttClient.loop();
}
