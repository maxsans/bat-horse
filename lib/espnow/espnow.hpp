#pragma once

#include <esp_err.h>
#include <functional>
#include <vector>
#include <string>

namespace espNow
{
    esp_err_t init(std::function<void(std::string, std::string)> callbackFunction);
    esp_err_t start(void);
    esp_err_t sendDataSensor(std::string &data);
    void setReadyToSend(bool value);
} // namespace espNow