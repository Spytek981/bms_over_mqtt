#include "espressif/esp_common.h"
#include <ssid_config.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <dhcpserver.h>
#include <lwip/api.h>
#include <string.h>
#include "datamanager.h"
#define WIFI_CLIENT_MODE 1

extern SemaphoreHandle_t wifi_alive;


static void  wifi_task(void *pvParameters)
{
    uint8_t status  = 0;
    uint8_t retries = 30;
    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    TSpineConfigDataStruct actualConfig;

    DATAMANAGER_getSpineData(&actualConfig);

    if(actualConfig.wifiMode)
    // if(DATAMANAGER_getWiFiMode())
    // if(0)
    /* WIFI work in client mode*/
    {
        
        strcpy(config.ssid, actualConfig.ssid);
        strcpy(config.password, actualConfig.password);;
        printf("WiFi CLIENT MODE: connecting to WiFi %s %s\n\r", config.ssid, config.password );
        sdk_wifi_set_opmode(STATION_MODE);
        sdk_wifi_station_set_config(&config);
        
        while(1)
        {
            while ((status != STATION_GOT_IP) && (retries)){
                status = sdk_wifi_station_get_connect_status();
                printf("%s: status = %d\n\r", __func__, status );
                if( status == STATION_WRONG_PASSWORD ){
                    printf("WiFi: wrong password\n\r");
                    break;
                } else if( status == STATION_NO_AP_FOUND ) {
                    printf("WiFi: AP not found\n\r");
                    break;
                } else if( status == STATION_CONNECT_FAIL ) {
                    printf("WiFi: connection failed\r\n");
                    break;
                }
                vTaskDelay( 1000 / portTICK_PERIOD_MS );
                --retries;
            }
            if (status == STATION_GOT_IP) {
                printf("WiFi: Connected\n\r");
                DATAMANAGER_setSpineStatusBit(0, 1);
                xSemaphoreGive( wifi_alive );
                taskYIELD();
            }

            while ((status = sdk_wifi_station_get_connect_status()) == STATION_GOT_IP) {
                xSemaphoreGive( wifi_alive );
                taskYIELD();
            }
            printf("WiFi: disconnected\n\r");
            DATAMANAGER_setSpineStatusBit(0, 0);
            sdk_wifi_station_disconnect();
            goto connection_failed;
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
        }
    }
    else
    /* WIFI work in AP mode*/
    {
    connection_failed:
        printf("WiFi AP MODE: starting AP\n\r");
        sdk_wifi_set_opmode(SOFTAP_MODE);
        struct ip_info ap_ip;
        IP4_ADDR(&ap_ip.ip, 10, 0, 6, 1);
        IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
        IP4_ADDR(&ap_ip.netmask, 255, 255, 255, 0);
        sdk_wifi_set_ip_info(1, &ap_ip);

        struct sdk_softap_config ap_config = 
        {
            .ssid = AP_SSID,
            .ssid_hidden = 0,
            .channel = 3, 
            .ssid_len = strlen(AP_SSID),
            .authmode = AUTH_WPA_WPA2_PSK, 
            .password = AP_PSK, 
            .max_connection = 3, 
            .beacon_interval = 100, 
        };
        sdk_wifi_softap_set_config(&ap_config);

        ip_addr_t first_client_ip;
        IP4_ADDR(&first_client_ip, 10, 0, 6, 10);
        dhcpserver_start(&first_client_ip, 3);

        while(1)
        {
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
        }
        
    }
    
}
static const char * const auth_modes [] = {
    [AUTH_OPEN]         = "Open",
    [AUTH_WEP]          = "WEP",
    [AUTH_WPA_PSK]      = "WPA/PSK",
    [AUTH_WPA2_PSK]     = "WPA2/PSK",
    [AUTH_WPA_WPA2_PSK] = "WPA/WPA2/PSK"
};
static void scan_done_cb(void *arg, sdk_scan_status_t status)
{
    char ssid[33]; // max SSID length + zero byte

    if (status != SCAN_OK)
    {
        printf("Error: WiFi scan failed\n");
        return;
    }

    struct sdk_bss_info *bss = (struct sdk_bss_info *)arg;
    // first one is invalid
    bss = bss->next.stqe_next;

    printf("\n----------------------------------------------------------------------------------\n");
    printf("                             Wi-Fi networks\n");
    printf("----------------------------------------------------------------------------------\n");

    while (NULL != bss)
    {
        size_t len = strlen((const char *)bss->ssid);
        memcpy(ssid, bss->ssid, len);
        ssid[len] = 0;

        printf("%32s (" MACSTR ") RSSI: %02d, security: %s\n", ssid,
            MAC2STR(bss->bssid), bss->rssi, auth_modes[bss->authmode]);

        bss = bss->next.stqe_next;
    }
}

static void scan_done_cbNEW(void *arg, sdk_scan_status_t status)
{
    char ssid[33]; // max SSID length + zero byte

    if (status != SCAN_OK)
    {
        printf("Error: WiFi scan failed\n");
        return;
    }

    struct sdk_bss_info *bss = (struct sdk_bss_info *)arg;
    // first one is invalid
    bss = bss->next.stqe_next;

    printf("\n----------------------------------------------------------------------------------\n");
    printf("                             Wi-Fi networks\n");
    printf("----------------------------------------------------------------------------------\n");

    while (NULL != bss)
    {
        if(!strcmp(bss->ssid, "laserX"))
        {
            size_t len = strlen((const char *)bss->ssid);
            memcpy(ssid, bss->ssid, len);
            ssid[len] = 0;

            printf("%s RSSI: %02d\n", ssid, bss->rssi);
        }

        bss = bss->next.stqe_next;
    }
}

void WIFI_scanNetwork(void)
{
    printf("Start to scan network\n");
    sdk_wifi_station_scan(NULL, scan_done_cbNEW);
    printf("FInished scanning\n");
}

int WIFI_init(void)
{
    // struct ip_info ap_ip;

    // printf("WIFI_init()\n");

    //         IP4_ADDR(&ap_ip.ip, 172, 16, 0, 1);
    //         IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
    //         IP4_ADDR(&ap_ip.netmask, 255, 255, 0, 0);
    //         sdk_wifi_set_ip_info(1, &ap_ip);

    //         struct sdk_softap_config ap_config = {
    //             .ssid = AP_SSID,
    //             .ssid_hidden = 0,
    //             .channel = 3,
    //             .ssid_len = strlen(AP_SSID),
    //             .authmode = AUTH_WPA_WPA2_PSK,
    //             .password = AP_PSK,
    //             .max_connection = 3,
    //             .beacon_interval = 100,
    //         };
    //         sdk_wifi_softap_set_config(&ap_config);

    //         ip_addr_t first_client_ip;
    //         IP4_ADDR(&first_client_ip, 172, 16, 0, 2);
    //         dhcpserver_start(&first_client_ip, 4);
    //         dhcpserver_set_dns(&ap_ip.ip);
    //         dhcpserver_set_router(&ap_ip.ip);

    if(xTaskCreate(&wifi_task, "wifi_task",  512, NULL, 2, NULL) == pdPASS)
        return true;
    else
        return false;
}