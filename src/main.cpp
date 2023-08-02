/*
 * @name Bat-Horse
 *
 * @version 1.0.1
 *
 * @description Motion capture for horse
 *
 * @author Maxsans and Charls
 *
 */
#include "globals.hpp"
#include "flash.hpp"
#include "espnow.hpp"
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
#include <string>
#include <iostream>

static const char *TAG = "Main";

void callback(std::string name, std::string src)
{
  printf("ok callback ");
}

extern "C" void app_main(void)
{
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  printf("\n\e[1;35m---------------------------------------------------------------------------------------------------------\e[0m\n");
  printf("\e[1;35m|\e[1;34m\t\t\t\t\t\t\t \t\t\t\t\t\t\e[1;35m|\e[0m\n");
  printf("\e[1;35m|\e[1;34m\t\t\t\t\t\t\t \t\t\t\t\t\t\e[1;35m|\e[0m\n");
  printf("\e[1;35m|\e[1;34m\t\t\t\t\t\tESP Start \t\t\t\t\t\t\e[1;35m|\e[0m\n");
  printf("\e[1;35m|\e[1;34m\t\t\t\t\t\t\t \t\t\t\t\t\t\e[1;35m|\e[0m\n");
  printf("\e[1;35m|\e[1;34m\t\t\t\t\t\t\t \t\t\t\t\t\t\e[1;35m|\e[0m\n");
  printf("\e[1;35m---------------------------------------------------------------------------------------------------------\e[0m\n");

  ESP_LOGI(TAG, "ESP Start");

  flash::init();

  wifi::init(static_cast<wifi::wifiMode>(WIFI_MODE));
  wifi::start();

  espNow::init(&callback);
  espNow::start();
  int i = 0;
#if !IS_BASE
  for (;;)
  {
    std::string myString = std::to_string(i);
    espNow::sendDataSensor(myString);
    vTaskDelay(7 / portTICK_PERIOD_MS);
    i++;
  }
#endif
}
