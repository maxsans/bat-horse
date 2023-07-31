/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sdkconfig.h>
#include <stdio.h>
#include <cmath>
#include <driver/i2c.h>

#include "mainMPU.hpp"

// TaskHandle_t taskHandle1 = NULL;

extern "C" void app_main()
{
  vTaskDelay(4000 / portTICK_PERIOD_MS);
  // On lance la tache qui va recupérer les données du capteur
  xTaskCreate(
      startMPU,   /* Task function. */
      "startMPU", /* name of task. */
      4048,       /* Stack size of task */
      NULL,       /* parameter of the task */
      5,          /* priority of the task */
      NULL);      /* Task handle to keep track of created task */
}
