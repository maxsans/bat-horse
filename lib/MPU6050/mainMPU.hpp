#pragma once

#include <functional>
#include <string>

namespace mpu
{
    /**
     * @brief initalize the mpu driver
     *
     * @return if succesful retunr ESP_OK, ESP_ERR otherwise
     */
    esp_err_t init(std::function<void(std::string)> callBack);

    /**
     * @brief start the mpu driver
     *
     * @return if succesful retunr ESP_OK, ESP_ERR otherwise
     */
    esp_err_t start(void);

    /**
     * @brief set fast motion capture
     *
     * @param enabled enable fast motion capture
     *
     * @return if succesful return ESP_OK, ESP_ERR otherwise
     */
    esp_err_t setFastMotionCapture(bool enabled);
} // namespace mpu
