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
TSpineConfigDataStruct SpineData;


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
    spineData->groups = 0x0001;
}

void DATAMANAGER_init(void)
{
    printf("%s\n", __func__);
    esp_spiffs_init();
    if (esp_spiffs_mount() != SPIFFS_OK) {
        printf("Error mount SPIFFS\n");
    }
    if(!DATAMANAGER_readSpineData())
    // if(1)
    {
        printf("Failed to read configuration. Init with default data, and save to file!\n");
        spineData_init(&SpineData);
        DATAMANAGER_saveSpineData();
    }

    return;
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

int8_t DATAMANAGER_setSpineData(TSpineConfigDataStruct *newdata )
{
    memcpy(&SpineData, newdata, sizeof(TSpineConfigDataStruct));
    printf("%s - new data saved!\n", __func__);
    DATAMANAGER_printSpineData();
    DATAMANAGER_saveSpineData();
    return true;
};

int8_t DATAMANAGER_getSpineData(TSpineConfigDataStruct *dataContainer )
{
    memcpy(dataContainer, &SpineData, sizeof(TSpineConfigDataStruct));
    printf("%s - data readed!\n", __func__);
    return true;
}

int8_t DATAMANAGER_saveSpineData(void)
{
    printf("%s\n", __func__);

    int fd = open(SPINE_DATA_FILE_NAME, O_WRONLY|O_CREAT, 0);
    if (fd < 0) {
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
    if (fd < 0) {
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