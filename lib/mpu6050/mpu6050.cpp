#include <Arduino.h>
#include "../../include/globals.hpp"
#include "inv_mpu.hpp"
#include "inv_mpu_dmp_motion_driver.hpp"
#include "mpu6050.hpp"
#include <Wire.h>

#define MAX_RETRY 10

// MPU interrupt pin
const int sensorIntPin = MPU_INT_PIN;
const int sampleRate = SAMPLE_RATE;
static TaskHandle_t *mpuSendDataTask;

int initSensor()
{
    Serial.printf("Initializing sensor\n");
    Wire.begin();

    int status;
    int retry = 1;
    while ((status = mpu_init(0)) != 0)
    {
        Serial.printf("mpu_init failed. Status: %d. Retry n°%d/%d\n", status, retry, MAX_RETRY);
        if (retry >= MAX_RETRY)
        {
            return 1;
        }
        else
        {
            retry++;
            delay(500);
        }
    }

    retry = 1;
    // Enable accelerometer and gyro sensors
    while (mpu_set_sensors(INV_XYZ_ACCEL | INV_XYZ_GYRO) != 0)
    {
        Serial.printf("mpu_set_sensors failed. Retry n°%d/%d\n", retry, MAX_RETRY);
        if (retry >= MAX_RETRY)
        {
            return 1;
        }
        else
        {
            retry++;
            delay(500);
        }
    }

    retry = 1;
    while (mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL) != 0)
    {
        Serial.printf("mpu_configure_fifo failed. Retry n°%d/%d\n", retry, MAX_RETRY);
        if (retry >= MAX_RETRY)
        {
            return 1;
        }
        else
        {
            retry++;
            delay(500);
        }
    }

    retry = 1;
    while (mpu_set_sample_rate(100) != 0)
    {
        Serial.printf("mpu_set_sample_rate failed. Retry n°%d/%d\n", retry, MAX_RETRY);
        if (retry >= MAX_RETRY)
        {
            return 1;
        }
        else
        {
            retry++;
            delay(500);
        }
    }
    Serial.printf("Uploading DMP firmware.\n");

    retry = 1;
    // Load DMP firmware
    while (dmp_load_motion_driver_firmware() != 0)
    {
        Serial.printf("dmp_load_motion_driver_firmware failed. Retry n°%d/%d\n", retry, MAX_RETRY);
        if (retry >= MAX_RETRY)
        {
            return 1;
        }
        else
        {
            retry++;
            delay(500);
        }
    }

    retry = 1;
    // Enable DMP features
    while (dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
                              DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL |
                              DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL) != 0)
    {
        Serial.printf("dmp_enable_feature failed. Retry n°%d/%d\n", retry, MAX_RETRY);
        if (retry >= MAX_RETRY)
        {
            return 1;
        }
        else
        {
            retry++;
            delay(500);
        }
    }

    retry = 1;
    while (mpu_set_accel_fsr(4) != 0)
    {
        Serial.printf("mpu_set_accel_fsr failed. Retry n°%d/%d\n", retry, MAX_RETRY);
        if (retry >= MAX_RETRY)
        {
            return 1;
        }
        else
        {
            retry++;
            delay(500);
        }
    }

    if (dmp_set_fifo_rate(sampleRate))
    {
        Serial.printf("dmp_set_fifo_rate failed.\n");
    }

    retry = 1;
    // Start DMP processing
    if (mpu_set_dmp_state(1) != 0)
    {
        Serial.printf("mpu_set_dmp_state failed. Retry n°%d/%d\n", retry, MAX_RETRY);
        if (retry >= MAX_RETRY)
        {
            return 1;
        }
        else
        {
            retry++;
            delay(500);
        }
    }

    Serial.printf("MPU/DMP running\n");
    return 0;
}

void initSensorInterrupt(TaskHandle_t *sendDataTask)
{
    mpuSendDataTask = sendDataTask;
    Serial.printf("Initializing sensor interrupt\n");

    pinMode(sensorIntPin, INPUT_PULLUP);
    attachInterrupt(
        digitalPinToInterrupt(sensorIntPin), []
        { xTaskNotifyFromISR(*mpuSendDataTask, 0, eNoAction, NULL); },
        FALLING);
}
