#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"



QueueHandle_t Reading_queue;
QueueHandle_t result_queue;

 SemaphoreHandle_t  binary_Sema;
 SemaphoreHandle_t  Mutex;

 #define Blue_LED 25
 #define Green_LED 26
 #define Red_LED 27
 #define Buzzer 33
 #define PushButton 32

typedef struct {
    int uid_card ;
    char string[8];
    
} type;

type arr[] = {
    {0x02 , "Ahmed"},
    {0x03 , "Ali"},
    {0x04 , "Hassan"},
    {0x05 , "Mohamed"},
    {0x06 , "Omar"},
    {0x07 , "Youssef"},
    {0x08 , "Khaled"},
    {0x09 , "Tamer"},
    {0x10 , "Mahmoud"}
};

    typedef enum {ACCESS_DENIED  ,  ACCESS_GRANTED  , Exit}result_type_t;

    typedef struct {
       result_type_t result;
    }result_t;



 void READ_RFID(void *pvParameters)
      {
         
         int uid ;
          result_t Result ; 
        while(1)
        {
            xQueueReceive(Reading_queue ,&uid , portMAX_DELAY );
            
            for(int i = 0 ; i < 9 ; i++)
            {
                if(uid == arr[i].uid_card)
                {
                   Result.result = ACCESS_GRANTED ; 
                   break ;
                }

                else 
                {
                    
                   Result.result = ACCESS_DENIED ;

                }
            }

            xQueueSend(result_queue , &Result , portMAX_DELAY );


           

        }
      
        
}





 void Push_Button(void *pvParameters)
 {
    result_t r ;
    while(1)
    {
        xSemaphoreTake(binary_Sema , portMAX_DELAY);

        r.result = Exit;
        
        xQueueSend(result_queue , &r , portMAX_DELAY);
        
    }
    
 }


 int counter = 0; 
 int Correct_Cards = 0 ;

 void Control(void *pvParameter)
 {
    result_t res ; 
    while(1)
    {
        xQueueReceive(result_queue , &res , portMAX_DELAY);
        counter++;
        if(res.result == ACCESS_GRANTED)
        {

            xSemaphoreTake(Mutex , portMAX_DELAY);
                oled_clear();
              oled_print("ACCESS GRANTED");
             
           xSemaphoreGive(Mutex);


           gpio_set_level(Green_LED , 1);
           gpio_set_level(Blue_LED , 1);
           gpio_set_level(Buzzer , 1);
           vTaskDelay(pdMS_TO_TICKS(100));
           gpio_set_level(Buzzer , 0);
           vTaskDelay(pdMS_TO_TICKS(2900));
           gpio_set_level(Green_LED , 0);
           gpio_set_level(Blue_LED , 0);
           
           Correct_Cards++;
           
        }

        else if (res.result == ACCESS_DENIED)
        {
              xSemaphoreTake(Mutex , portMAX_DELAY);
               
              printf("ACCESS_DENIED");
             
           xSemaphoreGive(Mutex);

            for(int i = 0 ; i < 3 ; i ++)
            {
                gpio_set_level(Red_LED , 1 );
                gpio_set_level(Buzzer , 1 );
                vTaskDelay(pdMS_TO_TICKS(1000));
                 gpio_set_level(Red_LED , 0 );
                gpio_set_level(Buzzer , 0 );
            }
        }

        else if(res.result == Exit)
        {

             xSemaphoreTake(Mutex , portMAX_DELAY);
            
              printf("Exit");
             
           xSemaphoreGive(Mutex);

            
        }
    }
 }




 void State(void *pvParameter)
 {
    while(1)
    {
        xSemaphoreTake(Mutex , portMAX_DELAY);
        
        
        printf("OK : ");
        printf(Correct_Cards);

        printf("No : ");
        printf(counter);

        xSemaphoreGive(Mutex);

    }
 }







 void IRAM_ATTR button_isr(void *arg)
 {
    xSemaphoreGiveFromISR(binary_Sema , NULL);
 }



 void app_main(void)
{

    gpio_set_direction(Red_LED , GPIO_MODE_OUTPUT);
    gpio_set_direction(Green_LED , GPIO_MODE_OUTPUT);
    gpio_set_direction(Blue_LED , GPIO_MODE_OUTPUT);
    gpio_set_direction(Buzzer , GPIO_MODE_OUTPUT);




    ////////////////// ISR /////////////////////
    
 
    gpio_set_direction(PushButton , GPIO_MODE_INPUT);
    gpio_set_pull_mode(PushButton , GPIO_PULLUP_ONLY);
    gpio_set_intr_type(PushButton , GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PushButton , button_isr , NULL);







    binary_Sema = xSemaphoreCreateBinary();
    Mutex =  xSemaphoreCreateMutex();
    

    Reading_queue = xQueueCreate(4 , sizeof(int));
    result_queue = xQueueCreate(4 , sizeof(result_t));

     
    
   xTaskCreate(READ_RFID , "READ_RFID" , 2048 , NULL , 3 , NULL);
    xTaskCreate(Push_Button , "Push_Button" , 2048 , NULL , 5 , NULL);
     xTaskCreate(Control , "Control" , 2048 , NULL , 4 , NULL);
       xTaskCreate(State , "State" , 2048 , NULL , 1 , NULL);

}

