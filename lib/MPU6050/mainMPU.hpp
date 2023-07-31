#pragma once


namespace mpu
{
    /**
     * @brief initalize the mpu driver
     *
     * @return if succesful retunr ESP_OK, ESP_ERR otherwise
     */
    esp_err_t init(void);

    /**
     * @brief start the mpu driver
     *
     * @return if succesful retunr ESP_OK, ESP_ERR otherwise
     */
    esp_err_t start(void);
} // namespace mpu
