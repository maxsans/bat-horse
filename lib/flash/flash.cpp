#include "flash.hpp"

#include <esp_check.h>
#include <esp_log.h>
#include <nvs_flash.h>

static const char *TAG = "FLASH";

namespace flash
{
    esp_err_t init()
    {
        // Initialize NVS
        ESP_LOGI(TAG, "Initialize Flash ...");

        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
            ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);
        ESP_LOGI(TAG, "Initialize Flash done");
        return ESP_OK;
    }
} // namespace flash
