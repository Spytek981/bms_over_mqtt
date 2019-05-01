#include "espressif/esp_common.h"
#include "esp/uart.h"
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

typedef struct SpineConfigDataStruct
{
    uint32_t statusRegister;
    uint8_t ssid[32];     /* Null terminated string */
    uint8_t password[64]; /* Null terminated string */
    uint8_t name[32];
}SpineConfigDataStruct;


SpineConfigDataStruct SpineData;

static void spineData_init(SpineConfigDataStruct *spineData)
{
    strcpy(spineData->name,  "My first spine module");
    strcpy(spineData->ssid, "laserX");
    strcpy(spineData->password, "a1s2d3f4");
    spineData->statusRegister = 0xA5A5A5A5;
}

void DATAMANAGER_init(void)
{
    spineData_init(&SpineData);
    return;
}

void DATAMANAGER_printSpineData(void)
{
    printf("name: %s\n", SpineData.name);
    printf("ssid: %s\n", SpineData.ssid);
    printf("password: %s\n", SpineData.password);
    printf("statusRegister: %s\n", SpineData.statusRegister);
}