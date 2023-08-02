#include "testTask.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <string>

#define MAX_SIZE 5
#define TAG "testTask"

QueueHandle_t xQueue;
// SemaphoreHandle_t mutex = xSemaphoreCreateMutex;

namespace test
{
    void init(void)
    {
        // xQueue = xQueueCreate(MAX_SIZE, sizeof(std::string));
        xQueue = xQueueCreate(MAX_SIZE, sizeof(char));
        ESP_LOGI(TAG, "Queue créée");
    }
} // namespace test

void sendTask1(void *pvParameters) // <- une tâche
{
    for (;;) // <- boucle infinie
    {
        xQueueSend(xQueue, "A", 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void sendTask2(void *pvParameters) // <- une tâche
{
    for (;;) // <- boucle infinie
    {
        xQueueSend(xQueue, "B", 0);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void recieveTask(void *pvParameters) // <- une tâche
{
    // std::string receivedValue;
    char receivedValue;
    BaseType_t status;

    for (;;) // <- boucle infinie
    {
        // xSemaphoreTake(mutex, portMAX_DELAY);

        ESP_LOGI(TAG, "Elements in queue : %d", uxQueueMessagesWaiting(xQueue));

        status = xQueueReceive(xQueue, &receivedValue, pdMS_TO_TICKS(100));
        if (status == pdPASS)
        {
            // ESP_LOGI(TAG, "recieve : %s;", receivedValue.c_str());
            ESP_LOGI(TAG, "recieve : %c;", receivedValue);
        }
        else
        {
            ESP_LOGI(TAG, "recieve : aucun");
        }

        // xSemaphoreGive(mutex);

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

void backTask(void *pvParameters)
{

    for (;;)
    {
        ESP_LOGI(TAG, "...........");
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}