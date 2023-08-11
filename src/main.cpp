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
#include "mainMPU.hpp"
#include "mqtt.hpp"

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
#include <cJSON.h>
#include <sstream>
#include <iomanip>

static const char *TAG = "Main";

void callbackDevicesBase(std::vector<espNow::SensorData> sensorList)
{
  printf("Callback devices connected :\n");
  for (auto sensorData : sensorList)
  {
    printf("\t- %s\n", sensorData.sensorName.c_str());
  }
  cJSON *jsonRoot = cJSON_CreateObject();
  for (auto sensorData : sensorList)
  {
    std::ostringstream stream;
    for (size_t i = 0; i < sensorData.sensorAddress.size(); ++i)
    {
      // Utilise std::setw et std::setfill pour formater chaque octet en hexadécimal avec deux chiffres
      stream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(sensorData.sensorAddress[i]);

      // Ajoute le caractère ':' sauf pour le dernier octet
      if (i < sensorData.sensorAddress.size() - 1)
      {
        stream << ":";
      }
    }
    std::string macAddrStr = stream.str();
    cJSON *jsonObject = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonObject, "id", sensorData.sensorId);
    cJSON_AddStringToObject(jsonObject, "name", sensorData.sensorName.c_str());
    cJSON_AddStringToObject(jsonObject, "address mac", macAddrStr.c_str());
    cJSON_AddItemToArray(jsonRoot, jsonObject);
  }
  char *jsonChar = cJSON_Print(jsonRoot);
  std::string jsonStr = jsonChar;
  std::string topic = "bat-horse/devices";

  mqtt::publish(topic, jsonStr);

  free(jsonChar);
  cJSON_Delete(jsonRoot);
}

void callbackMpu(std::string data)
{
  espNow::sendDataSensor(data);
}

void callback(std::string name, std::string data)
{
  printf("%s\n", data.c_str());
#if IS_BASE
  std::string topic = "bat-horse/" + name;
  mqtt::publish(topic, data);
#endif
}

void recordCallback(std::string topic, std::string data)
{
  printf("%s: %s\n", topic.c_str(), data.c_str());
  cJSON *jsonRoot = cJSON_Parse(data.c_str());
  if (jsonRoot == NULL)
  {
    ESP_LOGE(TAG, "failed to parse JSON root");
    return;
  }
  cJSON *jsonRecord = cJSON_GetObjectItem(jsonRoot, "record");
  if (jsonRecord == NULL)
  {
    ESP_LOGE(TAG, "failed to parse JSON record");
    return;
  }
  if (cJSON_IsBool(jsonRecord))
  {
    bool jsonBool = cJSON_IsTrue(jsonRecord);

    if (jsonBool)
    {
      espNow::startMeasurement();
    }
    else
    {
      espNow::stopMeasurement();
    }
  }
  else
  {
    ESP_LOGE(TAG, "Error parsing json bool in topic %s", topic.c_str());
  }
}

extern "C" void
app_main(void)
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
#if !IS_BASE
  mpu::init(callbackMpu);
  mpu::start();
#else
  mqtt::init();
  mqtt::start();
  std::string topicRecord = RECORD_TOPIC;
  mqtt::subscribe(topicRecord, recordCallback);
#endif

  espNow::init(callback, callbackDevicesBase);
  espNow::start();
}
