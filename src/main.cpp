/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <sdkconfig.h>

#include "mainMPU.hpp"

SemaphoreHandle_t xBinarySemaphore = NULL;
// TaskHandle_t taskHandle1 = NULL;

extern "C" void app_main()
{
  vTaskDelay(4000 / portTICK_PERIOD_MS);
  // Interruption

  // On lance la tache qui va recupérer les données du capteur
  mpu::init();
  mpu::start();
}
