#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_mpu.hpp>
#include <inv_mpu.hpp>
#include <inv_mpu_dmp_motion_driver.hpp>
#include <mqtt.hpp>



// WiFi settings
const char* ssid = "eir98829489";
const char* password = "ejfCu3cpgS";

// Server settings
const char* serverIP = "192.168.1.125";
const int serverPort = 5555;
const int localPort = 1025;

// Sensor settings
const int sensorID = 9;
const int sampleRate = 25;


const char* data_topic = "bat_horse/data-" + sensorID +;

// MPU interrupt pin
const int sensorIntPin = 10;

// Task queue for send_data tasks
static TaskHandle_t sendDataTask;

// Set as soon as we get an IP address
static bool gotIP = false;

static WiFiUDP udpClient;

void onWiFiEvent(WiFiEvent_t event);
void initWiFi();
int initSensor();
void initSensorInterrupt();
void sendDataHandler(void* parameter);
void heartbeatTick();

void setup() {
      delay(5000);

  Serial.begin(115200);

  Serial.printf("\n Sensor %d Startup! \n", sensorID);

  // Setup WiFi
  initWiFi();

  // Initialize MPU sensor
  if (initSensor()){
    Serial.printf("initSensor failed.\n");
    return;
  }

  // Initialize MPU sensor interrupt
  initSensorInterrupt();

  xTaskCreate(sendDataHandler, "sendDataTask", 4096, NULL, 1, &sendDataTask);
}

void loop() {
  // Do nothing here, work is done in tasks
}

void initWiFi() {
  Serial.printf("Initializing WiFi\n");

  WiFi.onEvent(onWiFiEvent);

  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.printf("Connecting to WiFi...\n");
  }
}

int initSensor() {
  Serial.printf("Initializing sensor\n");

  Wire.begin();

  int status;
  if ((status = mpu_init(0)) != 0) {
    Serial.printf("mpu_init failed. Status: %d\n", status);
    return 1;
  }

  // Enable accelerometer and gyro sensors
  if (mpu_set_sensors(INV_XYZ_ACCEL | INV_XYZ_GYRO)) {
    Serial.printf("mpu_set_sensors failed\n");
    return 1;
  }

  if (mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL)) {
    Serial.printf("mpu_configure_fifo failed\n");
    return 1;
  }

  if (mpu_set_sample_rate(100)) {
    Serial.printf("mpu_set_sample_rate failed\n");
    return 1;
  }

  Serial.printf("Uploading DMP firmware\n");

  // Load DMP firmware
  if (dmp_load_motion_driver_firmware()) {
    Serial.printf("dmp_load_motion_driver_firmware failed\n");
    return 1;
  }

  // Enable DMP features
  if (dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
                         DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL |
                         DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL)) {
    Serial.printf("dmp_enable_feature failed\n");
    return 1;
  }

  if (mpu_set_accel_fsr(4)) {
    Serial.printf("mpu_set_accel_fsr failed\n");
    return 1;
  }

  if (dmp_set_fifo_rate(sampleRate)) {
    Serial.printf("dmp_set_fifo_rate failed\n");
  }

  // Start DMP processing
  if (mpu_set_dmp_state(1)) {
    Serial.printf("mpu_set_dmp_state failed\n");
    return 1;
  }

  Serial.printf("MPU/DMP running\n");

  return 0;
}

void initSensorInterrupt() {
  Serial.printf("Initializing sensor interrupt\n");

  pinMode(sensorIntPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorIntPin), [] {
    if (gotIP) {
      xTaskNotifyFromISR(sendDataTask, 0, eNoAction, NULL);
    }
  }, FALLING);
}

void sendDataHandler(void* parameter) {
  udpClient.begin(localPort);

  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (!gotIP) {
      continue;
    }

    short gyro[3], accel[3], sensors;
    unsigned char more;
    long quat[4];
    unsigned long timestamp;

    if (dmp_read_fifo(gyro, accel, quat, &timestamp, &sensors, &more)) {
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


// #ifdef MQTT
String data_str;

for (int i = 0; i < 12; ++i) {
    data_str += String(data[i]);
}

mqtt::publish(data_topic, data_str);
// #endif
#ifdef UDP
    udpClient.beginPacket(serverIP, serverPort);
    udpClient.write((uint8_t*)data, 12 * sizeof(long));
    udpClient.endPacket();
#endif

  }
}

void onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.printf("Event: SYSTEM_EVENT_STA_GOT_IP\n");
      gotIP = true;
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.printf("Event: SYSTEM_EVENT_STA_CONNECTED\n");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.printf("Event: SYSTEM_EVENT_STA_DISCONNECTED\n");
      gotIP = false;
      break;
    default:
      Serial.printf("Unexpected WiFi event: %d\n", event);
      break;
  }
}
