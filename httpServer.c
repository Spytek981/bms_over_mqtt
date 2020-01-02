
#include <espressif/esp_common.h>
#include <esp8266.h>
#include <esp/uart.h>
#include <string.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>
#include <httpd/httpd.h>

#include "datamanager.h"


enum ssiTagIndexes 
{
    ssiTag_dev_name,
    ssiTag_net_ssid,
    ssiTag_net_password,
    ssiTag_save_Err_Details,
    ssiTag_net_apmode,
};


uint8_t saveErrorTracebackBuffer[64] = "To jest komunikat testowy";


int32_t ssi_handler(int32_t iIndex, char *pcInsert, int32_t iInsertLen)
{
    printf("%s %d", __func__, iIndex);
    switch (iIndex) {
        case ssiTag_dev_name:
        {
            printf("name\n");
            TSpineConfigDataStruct tempData;
            if(DATAMANAGER_getSpineData(&tempData))
            {
                // snprintf(pcInsert, iInsertLen, tempData.name);
                snprintf(pcInsert, iInsertLen, "test");
            }
            else
            {
                snprintf(pcInsert, iInsertLen, "UNDEFINED!");
            }
            break;
        }
        case ssiTag_net_ssid:
        {
            printf("ssid\n");
            TSpineConfigDataStruct tempData;
            if(DATAMANAGER_getSpineData(&tempData))
            {
                snprintf(pcInsert, iInsertLen, tempData.ssid);
            }
            else
            {
                snprintf(pcInsert, iInsertLen, "UNDEFINED!");
            }
            break;
        }
        case ssiTag_net_password:
        {
            printf("pass\n");
            TSpineConfigDataStruct tempData;
            if(DATAMANAGER_getSpineData(&tempData))
            {
                snprintf(pcInsert, iInsertLen, tempData.password);
            }
            else
            {
                snprintf(pcInsert, iInsertLen, "UNDEFINED!");
            }
            break;
        }
        case ssiTag_save_Err_Details:
        {
            snprintf(pcInsert, iInsertLen, saveErrorTracebackBuffer);
            break;
        }
        case ssiTag_net_apmode:
        {
            printf("apmode\n");
            TSpineConfigDataStruct tempData;
            if(DATAMANAGER_getSpineData(&tempData))
            {
                snprintf(pcInsert, iInsertLen, "%d", tempData.wifiMode);
            }
            else
            {
                snprintf(pcInsert, iInsertLen, "UNDEFINED!");
            }
            break;
        }
        default:
            snprintf(pcInsert, iInsertLen, "N/A");
            break;
    }

    /* Tell the server how many characters to insert */
    return (strlen(pcInsert));
}


char *test_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    for (int i = 0; i < iNumParams; i++) {
        printf("param_%d: %s, value: %s\n", i, pcParam[i], pcValue[i]);
    }
    return "/savedOK.html";
}

char *commitData_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // if(iNumParams < 3)
    // {
    //     strcpy(saveErrorTracebackBuffer, "Not enought parameters");
    //     return "/savedERR.shtml";
    // }
    TSpineConfigDataStruct newSpineConfiguration;
    DATAMANAGER_getSpineData(&newSpineConfiguration);
    for (int i = 0; i < iNumParams; i++) {
        printf("param_%d: %s, value: %s\n", i, pcParam[i], pcValue[i]);
        if(!strcmp(pcParam[i], "set_devname")) strcpy(&newSpineConfiguration.name, pcValue[i]);
        if(!strcmp(pcParam[i], "set_netssid")) strcpy(&newSpineConfiguration.ssid, pcValue[i]);
        if(!strcmp(pcParam[i], "set_netpass")) strcpy(&newSpineConfiguration.password, pcValue[i]);
        if(!strcmp(pcParam[i], "set_apmode")) newSpineConfiguration.wifiMode = atoi(pcValue[i]);
    }
    // printf("%s %s %s\n", newSpineConfiguration.name, newSpineConfiguration.ssid, newSpineConfiguration.password);
    if(DATAMANAGER_setSpineData(&newSpineConfiguration))
        return "/savedOK.html";
    else
    {
        strcpy(saveErrorTracebackBuffer, "DATAMANAGER_setSpineData failed");
        return "/savedERR.shtml";
    }
}

static void httpd_task(void *pvParameters)
{
    tCGI pCGIs[] = {
        {"/config",(tCGIHandler)commitData_cgi_handler},
    };

    const char *pcConfigSSITags[] = {
        "devname", 
        "netssid",
        "netpass",
        "errdet",
        "apmode",
    };

    /* register handlers and start the server */
    http_set_cgi_handlers(pCGIs, sizeof (pCGIs) / sizeof (pCGIs[0]));
    http_set_ssi_handler((tSSIHandler) ssi_handler, pcConfigSSITags,
            sizeof (pcConfigSSITags) / sizeof (pcConfigSSITags[0]));
    httpd_init();

    for (;;);
}

void HTTP_init(void)
{
    printf("%s\n", __func__);
    xTaskCreate(&httpd_task, "HTTP Daemon", 256, NULL, 2, NULL);
}