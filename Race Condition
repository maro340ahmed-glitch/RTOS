
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

char sharedText[20];

void Task1(void *pvParameters)
{
    while (1)
    {
        strcpy(sharedText, "HELLO");
        vTaskDelay(pdMS_TO_TICKS(10));

        printf("Task 1: %s\n", sharedText);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void Task2(void *pvParameters)
{
    while (1)
    {
        strcpy(sharedText, "WORLD");
        vTaskDelay(pdMS_TO_TICKS(10));

        printf("Task 2: %s\n", sharedText);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    xTaskCreate(Task1, NULL, 2048, NULL, 1, NULL);
    xTaskCreate(Task2, "Task2", 2048, NULL, 1, NULL);
}
