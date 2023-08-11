/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <cJSON.h>
#include <sdkconfig.h>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <driver/i2c.h>
#include <esp_check.h>
#include <driver/gpio.h>
#include <esp_timer.h>
#include <esp_sleep.h>

#include "mpu6050.hpp"
#include "kalmanfilter.hpp"
#include "mainMPU.hpp"
#include "../../include/globals.hpp"
#include "espnow.hpp"

const static char *TAG = "mpu6050";
const static uint8_t MAX_SIZE = 10;

std::function<void(std::string)> callBackMPU;

MPU6050 mpuDriver(GPIO_NUM_7, GPIO_NUM_6, I2C_NUM_0);

bool sleepMode = false;

static void
startMPU(void *pvParameters)
{
  double ax, ay, az, gx, gy, gz;
  double pitch, roll;
  double fpitch, froll;

  bool receivedValue;
  int loop = 0;
  esp_sleep_enable_timer_wakeup(1000);

  KALMAN pfilter(0.005);
  KALMAN rfilter(0.005);

  while (1)
  {
    if (espNow::getIsRecording())
    {
      if (mpuDriver.getInter())
      {
        sleepMode = false;
        loop = 0;
        // On recupère les données avec 2 chiffres apres la virgule
        ax = ceil(-mpuDriver.getAccX() * 100.0) / 100.0;
        ay = ceil(-mpuDriver.getAccY() * 100.0) / 100.0;
        az = ceil(-mpuDriver.getAccZ() * 100.0) / 100.0;
        gx = ceil(mpuDriver.getGyroX() * 100.0) / 100.0;
        gy = ceil(mpuDriver.getGyroY() * 100.0) / 100.0;
        gz = ceil(mpuDriver.getGyroZ() * 100.0) / 100.0;

        pitch = ceil(static_cast<float>(atan(ax / az) * 57.2958) * 100.0) / 100.0;
        roll = ceil(static_cast<float>(atan(ay / az) * 57.2958) * 100.0) / 100.0;
        fpitch = ceil(pfilter.filter(pitch, gy) * 100.0) / 100.0;
        froll = ceil(rfilter.filter(roll, -gx) * 100.0) / 100.0;

        // Create JSON object
        cJSON *json, *payload, *acc, *gyro;
        json = cJSON_CreateObject();
        payload = cJSON_CreateObject();
        // Add payload array to json
        cJSON_AddStringToObject(json, "name", SENSOR_NAME);
        cJSON_AddItemToObject(json, "payload", payload);
        // Add acc to payload array
        cJSON_AddItemToObject(payload, "acc", acc = cJSON_CreateObject());
        cJSON_AddItemToObject(acc, "x", cJSON_CreateNumber(ax));
        cJSON_AddItemToObject(acc, "y", cJSON_CreateNumber(ay));
        cJSON_AddItemToObject(acc, "z", cJSON_CreateNumber(az));

        // Add gryo to payload array
        cJSON_AddItemToObject(payload, "gyro", gyro = cJSON_CreateObject());
        cJSON_AddItemToObject(gyro, "x", cJSON_CreateNumber(gx));
        cJSON_AddItemToObject(gyro, "y", cJSON_CreateNumber(gy));
        cJSON_AddItemToObject(gyro, "z", cJSON_CreateNumber(gz));

        // Add other data to payload array
        cJSON_AddItemToObject(payload, "pitch", cJSON_CreateNumber(pitch));
        cJSON_AddItemToObject(payload, "fpitch", cJSON_CreateNumber(fpitch));
        cJSON_AddItemToObject(payload, "roll", cJSON_CreateNumber(roll));
        cJSON_AddItemToObject(payload, "froll", cJSON_CreateNumber(froll));

        char *out;
        out = cJSON_Print(json);
        // Cast char to string
        std::string data(out);
        // Vider les var
        free(out);
        cJSON_Delete(json);
        // Callback function
        printf("%s\n", data.c_str());
        callBackMPU(data);
      }
      else if ((sleepMode && loop == 50) || (!sleepMode && loop == 10000))
      {
        loop = 0;
        sleepMode = true;
        // ESP_LOGI(TAG, "START sleep");
        // esp_light_sleep_start();
      }
      loop++;
      // vTaskDelay(100/portTICK_PERIOD_MS);
      portYIELD();
    }
    else
    {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelete(NULL);
}

namespace mpu
{

  esp_err_t init(std::function<void(std::string)> callBack)
  {
    // Init mpuDriver
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_RETURN_ON_FALSE(mpuDriver.init(), ESP_FAIL, TAG, "init failed!");
    ESP_LOGI(TAG, "init success!");

    callBackMPU = callBack;

    return ESP_OK;
  }

  esp_err_t start()
  {
    xTaskCreate(startMPU, "startMPU", 4048, NULL, configMAX_PRIORITIES - 1, NULL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    return ESP_OK;
  }

  esp_err_t setFastMotionCapture(bool enabled)
  {
    ESP_RETURN_ON_FALSE(mpuDriver.setFastMotionCapture(enabled), ESP_FAIL, TAG, "failed to set fast motion capture");
    return ESP_OK;
  }

} // namespace mpu
