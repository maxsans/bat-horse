/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>

#include <sdkconfig.h>
#include <stdio.h>
#include <cmath>
#include <driver/i2c.h>
#include <esp_check.h>
#include <driver/gpio.h>

#include "mpu6050.hpp"
#include "kalmanfilter.hpp"


const static char *TAG = "mpu6050";
const static uint8_t GPIO_INT_PIN = GPIO_NUM_9;
const static uint8_t MAX_SIZE = 10;


MPU6050 mpuDriver(GPIO_NUM_7, GPIO_NUM_6, I2C_NUM_0);

QueueHandle_t xQueue = xQueueCreate(MAX_SIZE, sizeof(bool));



static void
startMPU(void *pvParameters)
{
  float ax, ay, az, gx, gy, gz;
  float pitch, roll;
  float fpitch, froll;

  bool receivedValue;

  KALMAN pfilter(0.005);
  KALMAN rfilter(0.005);

  while (1)
  {
    if (xQueueReceive(xQueue, &receivedValue, 10000 / portTICK_PERIOD_MS) == pdPASS)
    {
      ax = -mpuDriver.getAccX();
      ay = -mpuDriver.getAccY();
      az = -mpuDriver.getAccZ();
      gx = mpuDriver.getGyroX();
      gy = mpuDriver.getGyroY();
      gz = mpuDriver.getGyroZ();
      pitch = static_cast<float>(atan(ax / az) * 57.2958);
      roll = static_cast<float>(atan(ay / az) * 57.2958);
      fpitch = pfilter.filter(pitch, gy);
      froll = rfilter.filter(roll, -gx);
      ESP_LOGI(TAG, "Acc: ( %.3f, %.3f, %.3f)", ax, ay, az);
      ESP_LOGI(TAG, "Gyro: ( %.3f, %.3f, %.3f)", gx, gy, gz);
      // ESP_LOGI("mpu6050", "Pitch: %.3f", pitch);
      // ESP_LOGI("mpu6050", "Roll: %.3f", roll);
      // ESP_LOGI("mpu6050", "FPitch: %.3f", fpitch);
      // ESP_LOGI("mpu6050", "FRoll: %.3f", froll);
    }
    else
    {
      ESP_LOGI(TAG, "Rien dans la queue");
    }
  }
  vTaskDelete(NULL);
}

void IRAM_ATTR gpio_isr_handler(void *arg)
{
  // Code de gestion de l'interruption
  bool cond = true;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(xQueue, &cond, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken == pdTRUE)
  {
    portYIELD_FROM_ISR();
  }
}

namespace mpu
{

  esp_err_t init()
  {
    // Init mpuDriver
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_RETURN_ON_FALSE(mpuDriver.init(), ESP_FAIL, TAG, "init failed!");
    // Init GPIO
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;         // Type d'interruption (front montant)
    io_conf.pin_bit_mask = (1ULL << GPIO_INT_PIN); // Numéro de la broche GPIO à utiliser
    io_conf.mode = GPIO_MODE_INPUT;                // Mode d'entrée
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;      // Désactiver la résistance de pull-up interne
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;   // Activer la résistance de pull-down interne
    ESP_RETURN_ON_ERROR(gpio_config(&io_conf), TAG, "failed to initalize the intr GPIO");
    ESP_LOGI(TAG, "init success!");
    return ESP_OK;
  }

  esp_err_t start()
  {
    xTaskCreate(startMPU, "startMPU", 4048, NULL, 5, NULL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Installer la fonction de gestion de l'interruption pour la broche GPIO
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    gpio_isr_handler_add(static_cast<gpio_num_t>(GPIO_INT_PIN), gpio_isr_handler, nullptr);
    return ESP_OK;
  }

} // namespace mpu
