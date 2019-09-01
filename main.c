#ifndef LWIP_HTTPD_SSI_INCLUDE_TAG
    #warning "TAG NIE ZDEFINIOWANY!"
    #define LWIP_HTTPD_SSI_INCLUDE_TAG 0
#else 
    #warning "Dupa - tag juz zdefiniowany"
#endif

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
#include "utils.h"
SemaphoreHandle_t wifi_alive;
QueueHandle_t publish_queue;
EventGroupHandle_t event_bus;


void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());
    printf("Mode after startup: %d\n", sdk_wifi_get_opmode());
    printf("SPINE ID: %s\n", get_my_id());
    event_bus = xEventGroupCreate();
    vSemaphoreCreateBinary(wifi_alive);
    publish_queue = xQueueCreate(16, sizeof(mqttMessageContainer));
    DATAMANAGER_init();
    WIFI_init();
    TERMINAL_init();
    MQTT_init();
    IO_init();
    uint8_t macadr[64];
    sdk_wifi_get_macaddr(0,macadr);
    printf("MAC %s\n", macadr);

    HTTP_init();
}
