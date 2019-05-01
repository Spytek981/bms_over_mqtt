#include <stdlib.h>
#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"


static void IO_blnkTask(void *pvParameters)
{
    gpio_enable(2, GPIO_OUTPUT);
    while(1) {
        gpio_write(2, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_write(2, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void IO_init(void)
{
    printf("%s\n", __func__);
    if(xTaskCreate(&IO_blnkTask, "IO_blnkTask", 256, NULL, 3, NULL) == pdPASS)
        return true;
    else
        return false;
}