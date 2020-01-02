#include <stdlib.h>
#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"
#include <semphr.h>
#include "mqtt.h"
#include "datamanager.h"
#define buildingID "91addab8-c059-11e9-9cb5-2a2ae2dbcce4"

extern QueueHandle_t publish_queue;


static char buttonState = 0;
static void button_pressed(void)
{
    printf("BUTTON PRESSED\n");
    mqtt_send_to_group("SETOUT\0");
    
}

static void button_released(void)
{
    printf("BUTTON RELEASED\n");
    // mqtt_spine_send_event(0);
    mqtt_send_to_group("RESETOUT\0");
}

static void IO_blnkTask(void *pvParameters)
{
    gpio_enable(2, GPIO_OUTPUT);
    static uint8_t currentAnimationStep = 0;
    static uint8_t previousInput1State = 0;
    uint8_t animation = 0x0F;
    gpio_enable(13, GPIO_OUTPUT);
    gpio_enable(4, GPIO_INPUT);
    gpio_set_pullup(4, true, true);
    while(1) {

        switch (DATAMANAGER_getSpineStatus() & 0x03)
        {
            case (0x00):
                animation = 0x07;
                break;

            case (0x01):
                //Wifi connected, mqqtt disconnected
                animation = 0x3F;
                break;

            
            case (0x02):
            case (0x03):
                //Wifi whatever, mqtt connected:
                animation = 0xFF;
                break;
            
            default:
                //error state
                animation = 0x01;
                break;
            

        }

        
        gpio_write(2, !(animation & 1<<currentAnimationStep++));
        if(currentAnimationStep >= 8)
            currentAnimationStep = 0;
        if(gpio_read(4) != previousInput1State)
        {
            previousInput1State = gpio_read(4);
            if(previousInput1State)
            {
                button_released();                
            }
            else
            {
                button_pressed();
            }
            
        }
        vTaskDelay(125 / portTICK_PERIOD_MS);

    }

}

void IO_setOut(uint8_t state)
{
    gpio_write(13, state);
}



void IO_init(void)
{
    printf("%s\n", __func__);
    if(xTaskCreate(&IO_blnkTask, "IO_blnkTask", 1024, NULL, 3, NULL) == pdPASS)
        return true;
    else

        return false;
}

