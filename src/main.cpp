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

String sensor_id_str = String(SENSOR_ID);

// MQTT settings
String topic_data = MQTT_TOPIC_URI + sensor_id_str;
String client_id = "sensor-client_" + sensor_id_str;

// Server settings
#if STATIC_ADDRESS
const char *serverIP = SERVER_IP;
#endif
const int serverPort = SERVER_PORT;
const int localPort = LOCAL_PORT;

// Sensor settings
const int sensorID = SENSOR_ID;

// Set as soon as we get an IP address
static bool gotIP = false;

static bool isInitSensor = false;

static WiFiUDP udpClient;
static Mqtt mqttClient(BROKER_URL, MQTT_PORT);

const int sensorIntPin = MPU_INT_PIN;
static bool startDetection = true;

void sendDataHandler(void *parameter);
void networkTask(void *parameter);

void setup()
{
  Serial.begin(9600);
  xTaskCreatePinnedToCore(networkTask, "networkTask", 4096, NULL, 2, NULL, 0);
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

    short gyro[3],
        accel[3], sensors;
    unsigned char more;
    long quat[4];
    unsigned long timestamp;

    if (dmp_read_fifo(gyro, accel, quat, &timestamp, &sensors, &more))
    {
      Serial.printf("read_fifo_failed \n");
      delay(10); // wait 10ms
      continue;
    }

    long data[12];
    data[0] = sensorID;
    data[1] = quat[0];
    data[2] = quat[1];
    data[3] = quat[3];
    data[4] = -quat[2];
    data[5] = accel[0];
    data[6] = accel[2];
    data[7] = -accel[1];
    data[8] = gyro[0];
    data[9] = gyro[2];
    data[10] = -gyro[1];
    data[11] = timestamp;
    for (auto d : data)
    {
      Serial.print(d);
      Serial.print(" | ");
    }
    Serial.println("");
    if (gotIP)
    {
#if MQTT
      mqttClient.publish(topic_data.c_str(), (const char *)data);
#endif
#if UDP
#if STATIC_ADDRESS
      udpClient.beginPacket(serverIP, serverPort);
#else
      String gatewayAddress = getGatewayAddress();
      udpClient.beginPacket(gatewayAddress.c_str(), serverPort);
#endif
      udpClient.write((uint8_t *)data, 12 * sizeof(long));
      udpClient.endPacket();
#endif
    }
  }
}

void setEspToSleep()
{
  Serial.printf("start sleep\n");
  if (isInitSensor)
  {
    mpu_set_sensors(0);
  }
  esp_sleep_enable_timer_wakeup(SLEEP_TIME);
  esp_deep_sleep_start();
}

void networkTask(void *parameter)
{
  initWiFi(&gotIP);
  bool previousStateGotIp = false;
  int retry = 0;
  for (;;)
  {

    if (previousStateGotIp != gotIP)
    {
      if (gotIP)
      {
        // Initialize MPU sensor
        if (initSensor())
        {
          Serial.printf("initSensor failed.\n");
          setEspToSleep();
        }

        // Initialize MPU sensor interrupt
        initSensorInterrupt(&sendDataTask);

        xTaskCreate(sendDataHandler, "sendDataTask", 4096, NULL, 1, &sendDataTask);

        isInitSensor = true;
        Serial.printf("Initializing done \n");
        Serial.printf("\nSensor %d Startup! \n", sensorID);
#if UDP
        udpClient.begin(localPort);
#endif
        retry = 0;
      }
      previousStateGotIp = gotIP;
    }
    else if (!gotIP && retry == RETRY_MAX_WIFI)
    {
      setEspToSleep();
    }
    else if (!gotIP)
    {
      Serial.printf("Wifi: Connection failed retry\n");
      retry++;
    }
    delay(1000);
  }
#if MQTT
  mqttClient.connect(client_id.c_str());
  for (;;)
  {
    mqttClient.loop();
    delay(10);
  }
#endif
  vTaskDelete(NULL);
}
