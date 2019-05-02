#include <espressif/esp_common.h>
#include <esp8266.h>
#include <esp/uart.h>
#include <string.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>
#include <httpd/httpd.h>


int32_t ssi_handler(int32_t iIndex, char *pcInsert, int32_t iInsertLen)
{
    switch (iIndex) {
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

static void httpd_task(void *pvParameters)
{
    tCGI pCGIs[] = {
        {"/config",(tCGIHandler)test_cgi_handler},
    };

    const char *pcConfigSSITags[] = {
        "test" // SSI_UPTIME
    };

    /* register handlers and start the server */
    http_set_cgi_handlers(pCGIs, sizeof (pCGIs) / sizeof (pCGIs[0]));
    http_set_ssi_handler((tSSIHandler) ssi_handler, pcConfigSSITags,
            sizeof (pcConfigSSITags) / sizeof (pcConfigSSITags[0]));
    // websocket_register_callbacks((tWsOpenHandler) websocket_open_cb,
    //         (tWsHandler) websocket_cb);
    httpd_init();

    for (;;);
}

void HTTP_init(void)
{
    printf("%s\n", __func__);
    xTaskCreate(&httpd_task, "HTTP Daemon", 256, NULL, 2, NULL);
}