#pragma once

namespace test
{
    void init(void);
} // namespace test

void sendTask1(void *pvParameters);
void sendTask2(void *pvParameters);
void recieveTask(void *pvParameters);
void backTask(void *pvParameters);