#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"


#define GREEN_LED 2
#define BLUE_LED 4
#define RED_LED 5
#define BUZZER 18



typedef struct {

   int uid ;
    char name[8];
} user;

  user arr[6] = {
    
    { 0x34, "Alice" },
    { 0x2E, "Bob" },
    { 0xC4, "Charlie" },
    { 0xD8, "David" },
    { 0x6A, "Eve" },
    { 0x9A , "Frank" }

  };

typedef enum { RESULT_ENTRY, RESULT_DENIED, RESULT_EXIT } result_type_t;

typedef struct {
    result_type_t type;
    char name[8];
} result_t;


QueueHandle_t result_queue;
QueueHandle_t uid_queue;

SemaphoreHandle_t mutex;
SemaphoreHandle_t Binary_semaphore;



void Access_Task(void *pvParameters)
{
   
    int  uid;

    result_t result; 
   
    
    while(1)
    {
         
        xQueueReceive(uid_queue, &uid, portMAX_DELAY);

        result.type = RESULT_DENIED;
        
        for(int i = 0 ; i < 6 ; i++)
        {
            if(uid == arr[i].uid)
            {
                result.type = RESULT_ENTRY;
                strcpy(result.name, arr[i].name);   
             
            }
                
        }

        xQueueSend(result_queue, &result, portMAX_DELAY);

    }
  


}


void button_Task(void *pvParameters)

{
   result_t result;
   
   while(1)
   {
      semaphoreTake(Binary_semaphore, portMAX_DELAY);

      
      result.type = RESULT_EXIT;    
      strcpy(result.name, "Exit");

     xQueueSend(result_queue, &result, portMAX_DELAY);
      
   }

}




void Feedback_Task(void *pvParameters)
{
    result_t result;

    while (1)
    {
        xQueueReceive(result_queue, &result, portMAX_DELAY);

        if (result.type == RESULT_ENTRY)        
        {
            gpio_set_level(GREEN_LED, 1);
            gpio_set_level(BLUE_LED, 1);

            gpio_set_level(BUZZER, 1);           
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_set_level(BUZZER, 0);

            xSemaphoreTake(mutex, portMAX_DELAY);
            printf("OLED: %s\n", result.name);   
            xSemaphoreGive(mutex);
        }
        else if (result.type == RESULT_DENIED)  
        {
            gpio_set_level(RED_LED, 1);

            for (int i = 0; i < 3; i++)         
            {
                gpio_set_level(BUZZER, 1);
                vTaskDelay(pdMS_TO_TICKS(200));
                gpio_set_level(BUZZER, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }

            xSemaphoreTake(mutex, portMAX_DELAY);
            printf("OLED: DENIED\n");
            xSemaphoreGive(mutex);
        }
        else if (result.type == RESULT_EXIT)      // زرار الخروج
        {
            xSemaphoreTake(mutex, portMAX_DELAY);
            printf("OLED: EXIT\n");
            xSemaphoreGive(mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(3000));          // اللمبات منوّرة 3 ثواني

        gpio_set_level(GREEN_LED, 0);             // اطفي كله
        gpio_set_level(BLUE_LED, 0);
        gpio_set_level(RED_LED, 0);
    }
}






void app_main(void)
{

    gpio_set_direction(GREEN_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLUE_LED,  GPIO_MODE_OUTPUT);
    gpio_set_direction(RED_LED,   GPIO_MODE_OUTPUT);
    gpio_set_direction(BUZZER,    GPIO_MODE_OUTPUT);

    uid_queue    = xQueueCreate(4, sizeof(int));
    result_queue = xQueueCreate(4, sizeof(result_t));

    Binary_semaphore = xSemaphoreCreateBinary();
    mutex            = xSemaphoreCreateMutex();

    xTaskCreate(button_Task,   "Button",   2048, NULL, 5, NULL);
    xTaskCreate(Feedback_Task, "Feedback", 2048, NULL, 4, NULL);
    xTaskCreate(Access_Task,   "Access",   2048, NULL, 2, NULL);
}





