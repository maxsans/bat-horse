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

#include "mpu6050.hpp"
#include "kalmanfilter.hpp"

TaskHandle_t taskHandle1, taskHandle2, taskHandle3, taskHandle4 = NULL;

static void mpu6050_task(void *pvParameters)
{
  MPU6050 mpu(GPIO_NUM_7, GPIO_NUM_6, I2C_NUM_0);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  if (!mpu.init())
  {
    ESP_LOGE("mpu6050", "init failed!");
    vTaskDelete(0);
  }
  ESP_LOGI("mpu6050", "init success!");

  float ax, ay, az, gx, gy, gz;
  float pitch, roll;
  float fpitch, froll;

  KALMAN pfilter(0.005);
  KALMAN rfilter(0.005);

  while (1)
  {

    ax = -mpu.getAccX();
    ay = -mpu.getAccY();
    az = -mpu.getAccZ();
    gx = mpu.getGyroX();
    gy = mpu.getGyroY();
    gz = mpu.getGyroZ();
    pitch = static_cast<float>(atan(ax / az) * 57.2958);
    roll = static_cast<float>(atan(ay / az) * 57.2958);
    fpitch = pfilter.filter(pitch, gy);
    froll = rfilter.filter(roll, -gx);
    ESP_LOGI("mpu6050", "Acc: ( %.3f, %.3f, %.3f)", ax, ay, az);
    ESP_LOGI("mpu6050", "Gyro: ( %.3f, %.3f, %.3f)", gx, gy, gz);
    ESP_LOGI("mpu6050", "Pitch: %.3f", pitch);
    ESP_LOGI("mpu6050", "Roll: %.3f", roll);
    ESP_LOGI("mpu6050", "FPitch: %.3f", fpitch);
    ESP_LOGI("mpu6050", "FRoll: %.3f", froll);
  }
  vTaskDelete(NULL);
}

extern "C" void app_main()
{
  vTaskDelay(4000 / portTICK_PERIOD_MS);
  xTaskCreatePinnedToCore(&mpu6050_task, "mpu6050_task", 4048, NULL, 5, NULL, 0);
}
