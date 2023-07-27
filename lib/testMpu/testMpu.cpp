#include "testMpu.hpp"
#include "mpu6050.hpp"
#include <esp_log.h>

#define TAG "testMpu"

namespace testMpu
{
	void init(void *pvParameters)
	{
		ESP_LOGI(TAG, "MPU init start");
		mpu6050_init();
		ESP_LOGI(TAG, "MPU init ok");
		vTaskDelete(NULL);
	}
} // namespace test
