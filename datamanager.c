#include "espressif/esp_common.h"
#include "esp/uart.h"
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "datamanager.h"
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
    printf("%s\n", __func__);
    spineData_init(&SpineData);
    return;
}

void DATAMANAGER_printSpineData(void)
{
    printf("name: %s\n", SpineData.name);
    printf("ssid: %s\n", SpineData.ssid);
    printf("password: %s\n", SpineData.password);
    printf("statusRegister: %08X\n", SpineData.statusRegister);
}

int8_t DATAMANAGER_setSpineData(struct SpineConfigDataStruct *newdata )
{
    memcpy(&SpineData, newdata, sizeof(SpineConfigDataStruct));
    printf("%s - new data saved!\n", __func__);
    return true;
};