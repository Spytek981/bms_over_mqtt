#include "espressif/esp_common.h"
#include "esp/uart.h"
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "fcntl.h"
#include "unistd.h"
#include "spiffs.h"
#include "esp_spiffs.h"
#include "utils.h"
#include "datamanager.h"
#include <timers.h>
#include "mqtt.h"

#define FIRMWARE_VERSION (0.1)
TSpineConfigDataStruct SpineData;
TimerHandle_t timer_cyclicDataSender;
extern QueueHandle_t publish_queue;

void DATAMANAGER_cyclicSendSpineData_clbk(TimerHandle_t xTimer);

static void spineData_init(TSpineConfigDataStruct *spineData)
{
    //strcpy(spineData->name,  "Spine");
    sprintf(spineData->name, "SPINE_%s", get_my_id());
    strcpy(spineData->ssid, "laserX");
    strcpy(spineData->password, "a1s2d3f4");
    spineData->statusRegister = 0xA5A5A5A5;
    spineData->wifiMode = 1;
    strcpy(spineData->event_onClose_topic, "");
    strcpy(spineData->event_onClose_msg, "");
    strcpy(spineData->event_onOpen_topic, "");
    strcpy(spineData->event_onOpen_msg, "");
    strcpy(spineData->buildingId, "00000000-0000-0000-0000-000000000000");
    spineData->groups = 0x0000;
}

void DATAMANAGER_init(void)
{
    printf("%s\n", __func__);
    esp_spiffs_init();
    if (esp_spiffs_mount() != SPIFFS_OK)
    {
        printf("Error mount SPIFFS\n");
    }
    if (!DATAMANAGER_readSpineData())
    // if(1)
    {
        printf("Failed to read configuration. Init with default data, and save to file!\n");
        spineData_init(&SpineData);
        DATAMANAGER_saveSpineData();
    }
    //init software timer to cyclic sending data
    
    timer_cyclicDataSender = xTimerCreate(
        "SpineData_cyclic_sender",
        pdMS_TO_TICKS(30000),
        pdTRUE,
        (void *)0,
        DATAMANAGER_cyclicSendSpineData_clbk);
    if(timer_cyclicDataSender == NULL)
    {
        printf("timer_cyclicDataSender init FAILED!!!!!");
    }
    else
    {
        printf("timer_cyclicDataSender init OK. Starting\n");
        xTimerStart(timer_cyclicDataSender, 0);
    }
    
}

void DATAMANAGER_printSpineData(void)
{
    printf("name: %s\n", SpineData.name);
    printf("ssid: %s\n", SpineData.ssid);
    printf("password: %s\n", SpineData.password);
    printf("statusRegister: %08X\n", SpineData.statusRegister);
    printf("wifimode: %d\n", SpineData.wifiMode);
    printf("event_onClose_topic: %s\n", SpineData.event_onClose_topic);
    printf("event_onClose_msg: %s\n", SpineData.event_onClose_msg);
    printf("event_onOpen_topic: %s\n", SpineData.event_onOpen_topic);
    printf("event_onOpen_msg: %s\n", SpineData.event_onOpen_msg);
    printf("groups: 0x%08X\n", SpineData.groups);
}

int8_t DATAMANAGER_setSpineData(TSpineConfigDataStruct *newdata)
{
    memcpy(&SpineData, newdata, sizeof(TSpineConfigDataStruct));
    printf("%s - new data saved!\n", __func__);
    DATAMANAGER_printSpineData();
    DATAMANAGER_saveSpineData();
    return true;
};

int8_t DATAMANAGER_getSpineData(TSpineConfigDataStruct *dataContainer)
{
    memcpy(dataContainer, &SpineData, sizeof(TSpineConfigDataStruct));
    printf("%s - data readed!\n", __func__);
    return true;
}

int8_t DATAMANAGER_saveSpineData(void)
{
    printf("%s\n", __func__);

    int fd = open(SPINE_DATA_FILE_NAME, O_WRONLY | O_CREAT, 0);
    if (fd < 0)
    {
        printf("Error opening file %s\n", SPINE_DATA_FILE_NAME);
        return false;
    }

    int written = write(fd, &SpineData, sizeof(SpineData));
    printf("Written %d bytes\n", written);

    close(fd);
    return true;
}
int8_t DATAMANAGER_readSpineData(void)
{
    printf("%s\n", __func__);

    int fd = open(SPINE_DATA_FILE_NAME, O_RDONLY, 0);
    if (fd < 0)
    {
        printf("Error opening file %s\n", SPINE_DATA_FILE_NAME);
        return false;
    }

    int written = read(fd, &SpineData, sizeof(SpineData));
    printf("Readed %d bytes\n", written);

    close(fd);
    return true;
}

int8_t DATAMANAGER_getWiFiMode(void)
{
    return SpineData.wifiMode;
}

void DATAMANAGER_cyclicSendSpineData_clbk(TimerHandle_t xTimer)
{
    
    printf("%s - sending!\n", __func__);
    TSpineConfigDataStruct actualConfig;
    DATAMANAGER_getSpineData(&actualConfig);
    
    char ActualConfigStr[256];
    sprintf(
        ActualConfigStr, 
        "{\"status\":%u,\"ssid\":\"%s\",\"password\":\"%s\",\"name\":\"%s\",\"wifiMode\":%d,\"uid\":\"%s\",\"version\":\"%1.2f\",\"groups\":%d}",
        actualConfig.statusRegister,
        actualConfig.ssid,
        actualConfig.password,
        actualConfig.name,
        actualConfig.wifiMode,
        get_my_id(),
        FIRMWARE_VERSION,
        actualConfig.groups
    );
    mqttMessageContainer message;
    sprintf(message.messageTopic, "%s/broadcast/%s/status/", BASE_TOPIC, get_my_id());
    sprintf(message.messagePayload, "%s", ActualConfigStr);
    xQueueSend(publish_queue, &message, 100);
}