#pragma once

#include <esp_err.h>
#include <functional>
#include <cJSON.h>
#include <string>

namespace mqtt
{
    /**
     * @brief initialize the MQTT socket
     *
     * @param mqttBroker The mqtt broker
     *
     * @return esp_err_t
     * - ESP_OK if successful,
     * - ESP_ERR otherwise
     */
    esp_err_t init();

    /**
     * @brief start the MQTT socket
     *
     * @return esp_err_t
     * - ESP_OK if successful,
     * - ESP_ERR otherwise
     */
    esp_err_t start(void);

    /**
     * @brief publish a message to the MQTT
     *
     * @param topic The MQTT topic
     * @param data The message
     * @return esp_err_t
     * - ESP_OK if successful,
     * - ESP_ERR otherwise
     */
    esp_err_t publish(std::string &topic, std::string &data);

    /**
     * @brief subscribe to the MQTT
     *
     * @param topic
     * @param callbackFunction
     * @return esp_err_t
     */
    esp_err_t subscribe(std::string &topic, std::function<void(std::string, std::string)> &callbackFunction);
} // namespace mqtt
