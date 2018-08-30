#include "espressif/esp_common.h"
#include "esp/uart.h"
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>

#include <semphr.h>

#include "terminal.h"
#include "wifi.h"
#include "mqtt.h"

SemaphoreHandle_t wifi_alive;
QueueHandle_t publish_queue;

void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    vSemaphoreCreateBinary(wifi_alive);
    publish_queue = xQueueCreate(3, PUB_MSG_LEN);
    WIFI_init();
    
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    TERMINAL_init();
    MQTT_init();
}
