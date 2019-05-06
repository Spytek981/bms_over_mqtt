#include "espressif/esp_common.h"
#include "esp/uart.h"
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>

#include "terminal.h"
#include "wifi.h"
#include "mqtt.h"
#include "io.h"
#include "datamanager.h"
#include "httpServer.h"

SemaphoreHandle_t wifi_alive;
QueueHandle_t publish_queue;
EventGroupHandle_t event_bus;


void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());
    printf("Mode after startup: %d\n", sdk_wifi_get_opmode());
    //event_bus = xEventGroupCreate();
    vSemaphoreCreateBinary(wifi_alive);
    publish_queue = xQueueCreate(3, PUB_MSG_LEN);
    DATAMANAGER_init();
    WIFI_init();
    TERMINAL_init();
    MQTT_init();
    IO_init();

    HTTP_init();
}
