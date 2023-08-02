/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <sdkconfig.h>
#include <string>

#include "mainMPU.hpp"

SemaphoreHandle_t xBinarySemaphore = NULL;

void callBack(std::string str)
{
  printf("data %s\n", str.c_str());
}

extern "C" void app_main()
{
  vTaskDelay(4000 / portTICK_PERIOD_MS);
  // On lance la tache qui va recupérer les données du capteur
  mpu::init(callBack);
  mpu::start();
}
