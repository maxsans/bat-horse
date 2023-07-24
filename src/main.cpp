/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "wifi.hpp"
#include <esp_chip_info.h>
#include <esp_err.h>
#include <esp_flash.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <sdkconfig.h>
#include <stdio.h>

static const char *TAG = "Main";

extern "C" void app_main(void)
{
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  ESP_LOGI(TAG, "ESP Start");
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

  wifi::init();
  wifi::start();
}
