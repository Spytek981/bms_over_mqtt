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
    // xQueueSend(publish_queue, "{\"msg_id\" : 1234, \"ack_topic\":\"/spytek/testy/ABDEF01234/1234/\", \"event\":1}", 0);

    // mqtt_spine_send_event(1);
    mqttMessageContainer message;   
    TSpineConfigDataStruct spineData;
    DATAMANAGER_getSpineData(&spineData);

    if(spineData.event_onClose_topic && spineData.event_onClose_msg)
    {
        sprintf(message.messageTopic, "%s/%s/%s/", BASE_TOPIC, buildingID, spineData.event_onClose_topic);
        sprintf(message.messagePayload, "%s", spineData.event_onClose_msg);
        xQueueSend(publish_queue, &message, 100);
    }
    else
    {
        printf("No message to send\n");
    }
    
}

static void button_released(void)
{
    printf("BUTTON RELEASED\n");
    // mqtt_spine_send_event(0);
}

static void IO_blnkTask(void *pvParameters)
{
    gpio_enable(2, GPIO_OUTPUT);
    gpio_enable(5, GPIO_OUTPUT);
    gpio_enable(4, GPIO_INPUT);
    gpio_set_pullup(4, true, true);
    while(1) {

        if(gpio_read(4) != buttonState)
        {
            buttonState = gpio_read(4);
            if (!buttonState)
            {
                button_pressed();
            }
            else
            {
                button_released();
            }
            
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);

    }
}

void IO_setOut(uint8_t state)
{
    gpio_write(5, !state);
}



void IO_init(void)
{
    printf("%s\n", __func__);
    if(xTaskCreate(&IO_blnkTask, "IO_blnkTask", 1024, NULL, 3, NULL) == pdPASS)
        return true;
    else
        return false;
}

