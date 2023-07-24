#pragma once

#include <esp_err.h>

namespace wifi {

esp_err_t init();
esp_err_t start();
} // namespace wifi