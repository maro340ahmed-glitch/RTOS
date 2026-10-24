#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

char sharedText[20];

SemaphoreHandle_t SemA;

void Task1(void *pvParameters)
{
    char state=0;
    while (1)
    {
        state = xSemaphoreTake(SemA, pdMS_TO_TICKS(0));
        if(state == pdTRUE )
        {
        strcpy(sharedText, "HELLO");
        printf("Task 1: %s\n", sharedText);

        xSemaphoreGive(SemA);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void Task2(void *pvParameters)
{
     char state=0;
    while (1)
    {
        state = xSemaphoreTake(SemA, pdMS_TO_TICKS(0));
        if(state == pdTRUE )
        {
        strcpy(sharedText, "Eman");
        printf("Task 2: %s\n", sharedText);

        xSemaphoreGive(SemA);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    SemA = xSemaphoreCreateCounting(2,1);

    if (SemA == pdFAIL)
    {
        printf("Failed to create Binary Semaphore!\n");
        return;
    }


    xTaskCreate(Task1, "Task1", 2048, NULL, 1, NULL);
    xTaskCreate(Task2, "Task2", 2048, NULL, 1, NULL);


///vTaskStartScheduler();

}

