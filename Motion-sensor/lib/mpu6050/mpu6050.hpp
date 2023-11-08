#pragma once
#include <Arduino.h>

int initSensor(void);

void initSensorInterrupt(TaskHandle_t *sendDataTask);
