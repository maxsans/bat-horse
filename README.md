# Motion sensor (ESP_32 + MPU6050)

The purpose of this sensor is to retrieve motion data from an MPU6050 sensor and transmit it to an MQTT broker or UDP server. The sensor includes a sleep function to switch off the sensor when not in use.


## Project Configuration

The `globals.h` file plays a pivotal role in customizing and configuring the project. Adjust the variables in this file to ensure the project functions seamlessly in your specific environment. Here's an overview of the key configuration variables:

### Sensor Settings

```c
#define SENSOR_ID 9
#define SENSOR_ID_STRING STRINGIFY(SENSOR_ID)
#define SAMPLE_RATE 25
```


### WiFi Settings

```c
#define SSID "Name"       
#define PASSWORD "12345678" 
```


### Network Transport

```c
#define MQTT false
#define UDP true
```


### Server UDP Settings

```c
#define SERVER_IP "192.168.137.123"
#define SERVER_PORT 5555
#define LOCAL_PORT 1025
```


### Server MQTT Settings

```c
#define BROKER_URL "192.168.137.123"
#define MQTT_PORT 1883
#define MQTT_TOPIC_URI "motion-capture/sensor-" SENSOR_ID_STRING
```


### MPU Configuration Pin

```c
#define MPU_SDA_PIN 6
#define MPU_SCL_PIN 7
#define SENSOR_INT_PIN 10
```


### Motion Detection

```c
#define PRECISION_DETECTION 100
```


### Sleep Configuration

```c
#define TIMEOUT_DETECTION 0 * 1000
#define SLEEP_TIME 60 * 1000000
```

## Difference between transmission by MQTT and by UDP

The MQTT protocol uses a different protocol to UDP. MQTT uses the TCP protocol, so packets are slow when sent at very high speed. There is a test carried out on an EMQX broker:

| MQTT             | UDP               |
|:-:|:-:|
| 250 messages/min | 1500 messages/min |

These values are only valid for a single sensor. So if you have two sensors, you will receive 500 messages/min on an MQTT broker and 3000 messages/min on a UDP server.


### Example Output

```
Sensor 9 Startup!
Initializing sensor
Uploading DMP firmware.
MPU/DMP running
Initializing sensor interrupt
Initializing done
Initializing WiFi
Unexpected WiFi event: 0
Unexpected WiFi event: 2
Event: SYSTEM_EVENT_STA_CONNECTED
Event: SYSTEM_EVENT_STA_GOT_IP
Connected to MQTT broker
```


