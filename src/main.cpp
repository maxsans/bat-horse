#include <Arduino.h>
#include <WiFi.h>
#include <esp_mpu.hpp>
#include <inv_mpu.hpp>
#include <cstdlib>
#include <inv_mpu_dmp_motion_driver.hpp>
#include "globals.hpp"
#include "wifi.hpp"
#include <mqtt.hpp>
#include "mpu6050.hpp"

static TaskHandle_t sendDataTask;

// Server settings
const char *serverIP = SERVER_IP;
const int serverPort = SERVER_PORT;
const int localPort = LOCAL_PORT;

// Sensor settings
const int sensorID = SENSOR_ID;
const char *data_topic = TOPIC_DATA;

// Set as soon as we get an IP address
static bool gotIP = false;

static bool detection = false;

static WiFiUDP udpClient;
static Mqtt mqttClient(BROKER_URL, MQTT_PORT);

const int sensorIntPin = SENSOR_INT_PIN;
static bool startDetection = true;

long previousData[11] = {0};
QueueHandle_t eventQueue;

void sendDataHandler(void *parameter);
void heartbeatTick();
void sleepTask(void *parameter);
void networkTask(void *parameter);

void setup()
{
  Serial.begin(115200);

  Serial.printf("\n Sensor %d Startup! \n", sensorID);

  xTaskCreatePinnedToCore(networkTask, "networkTask", 4096, NULL, 2, NULL, 0);

  // Initialize MPU sensor
  if (initSensor())
  {
    Serial.printf("initSensor failed.\n");
    return;
  }

  // Initialize MPU sensor interrupt
  initSensorInterrupt(&sendDataTask);

  xTaskCreate(sendDataHandler, "sendDataTask", 4096, NULL, 1, &sendDataTask);

  eventQueue = xQueueCreate(10, sizeof(int));
  xTaskCreatePinnedToCore(sleepTask, "SleepTask", 4096, NULL, 1, NULL, 0);

  Serial.printf("Initializing done\n");
}

void loop()
{
  // Do nothing here, work is done in tasks
}

void sendDataHandler(void *parameter)
{
  bool status;

  while (1)
  {

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    short int status = 0;
    mpu_get_int_status(&status);
    // Serial.printf("test\n");
    // if (status)
    // {
    //   Serial.printf("Intr");
    // }
    short gyro[3],
        accel[3], sensors;
    unsigned char more;
    long quat[4];
    unsigned long timestamp;

    if (dmp_read_fifo(gyro, accel, quat, &timestamp, &sensors, &more))
    {
      Serial.printf("read_fifo_failed \n");
      continue;
    }

    long data[12];
    data[0] = sensorID;
    data[1] = quat[0];
    data[2] = quat[1];
    data[3] = quat[2];
    data[4] = quat[3];
    data[5] = accel[0];
    data[6] = accel[1];
    data[7] = accel[2];
    data[8] = gyro[0];
    data[9] = gyro[1];
    data[10] = gyro[2];
    data[11] = timestamp;
    if (gotIP)
    {
// Serial.printf("Send data\n");
#if MQTT
      mqttClient.publish(data_topic, (const char *)data);
#endif
#if UDP
      udpClient.beginPacket(serverIP, serverPort);
      udpClient.write((uint8_t *)data, 12 * sizeof(long));
      udpClient.endPacket();
#endif
    }
    // Comparaison avec la version précédente
    if (abs(data[5] - previousData[5]) > PRECISION_DETECTION || abs(data[6] - previousData[6]) > PRECISION_DETECTION || abs(data[7] - previousData[7]) > PRECISION_DETECTION)
    {
      if (startDetection)
      {
        startDetection = false;
        memcpy(previousData, data, sizeof(data));
      }
      else
      {
        // Serial.printf("detection mouvement\n");
        if (!detection)
        {
          detection = true;
        }
        // Serial.println("Received event, printing data:");
        // for (int i = 0; i < 11; i++)
        // {
        //   Serial.print(data[i]);
        //   Serial.print(" ");
        // }
        // Serial.println();

        // Serial.println("Previous data:");
        // for (int i = 0; i < 11; i++)
        // {
        //   Serial.print(previousData[i]);
        //   Serial.print(" ");
        // }
        // Serial.println();
        int eventData = 1;
        xQueueSend(eventQueue, &eventData, 0);

        // Mise à jour de la version précédente des données
        memcpy(previousData, data, sizeof(data));
      }
    }
  }
}

void networkTask(void *parameter)
{
  while (!detection)
  {
    delay(1000); // wait 1s
  }
  initWiFi(&gotIP);
#if UDP
  udpClient.begin(localPort);
#endif
#if MQTT
  mqttClient.connect(CLIENT_ID);
#endif
  for (;;)
  {
    mqttClient.loop();
    delay(10);
  }
  vTaskDelete(NULL);
}

void sleepTask(void *parameter)
{
  int eventData;
  for (;;)
  {
    if (TIMEOUT_DETECTION)
    {
      if (xQueueReceive(eventQueue, &eventData, pdMS_TO_TICKS(TIMEOUT_DETECTION)) == pdFALSE)
      {

        detection = false;
        Serial.printf("start sleep\n");
        esp_sleep_enable_timer_wakeup(SLEEP_TIME);
        esp_deep_sleep_start();
      }
    }
  }

  // Si un événement est reçu, la tâche se termine
  vTaskDelete(NULL);
}
