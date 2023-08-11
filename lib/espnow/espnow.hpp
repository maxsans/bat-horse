#pragma once

#include <esp_err.h>
#include <functional>
#include <vector>
#include <string>

namespace espNow
{
    struct SensorData
    {
        int sensorId;
        std::string sensorName;
        std::vector<uint8_t> sensorAddress;
    };

    esp_err_t init(std::function<void(std::string, std::string)> callbackFunction, std::function<void(std::vector<SensorData>)> callbackFunctionConnectedSensor);
    esp_err_t start(void);
    esp_err_t sendDataSensor(std::string &data);
    void setReadyToSend(bool value);
    esp_err_t deletePeerAddress(std::vector<uint8_t> &macAddr);
    esp_err_t startMeasurement(void);
    esp_err_t stopMeasurement(void);
    bool getIsRecording(void);
} // namespace espNow