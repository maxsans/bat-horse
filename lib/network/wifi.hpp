#pragma once

#include <esp_err.h>

namespace wifi
{
    enum class wifiMode
    {
        WIFI_NULL,
        WIFI_STA,
        WIFI_AP,
        WIFI_APSTA
    };

    esp_err_t init(wifiMode mode);
    esp_err_t start();
} // namespace wifi