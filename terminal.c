/* Serial terminal example
 * UART RX is interrupt driven
 * Implements a simple GPIO terminal for setting and clearing GPIOs
 *
 * This sample code is in the public domain.
 */
#include "espressif/esp_common.h"
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <esp8266.h>
#include <esp/uart.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "mqtt.h"
#include "terminal.h"
#include "datamanager.h"
#include "wifi.h"


#define MAX_ARGC (10)

#define PROMPT "CMD> "

extern QueueHandle_t publish_queue;
static void cmd_help(uint32_t argc, char *argv[]);
static void cmd_mqttSendMessage(uint32_t argc, char *argv[]);
static void cmd_reboot(uint32_t argc, char *argv[]);
static void cmd_printSpineDAta(uint32_t argc, char *argv[]);
static void cmd_setSpineData(uint32_t argc, char *argv[]);
static void cmd_setOut(uint32_t argc, char *argv[]);
static void cmd_scan(uint32_t argc, char *argv[]);


static struct CMD_dictionary 
{
    uint8_t cmdName[16];
    void (*cmdHandler)(uint32_t argc, char*argv[]);
};

static struct CMD_dictionary CMDdict[]=
{
    "help", &cmd_help,
    "sl", &cmd_mqttSendMessage,
    "reboot", &cmd_reboot,
    "psdata", &cmd_printSpineDAta,
    "ssdata", &cmd_setSpineData,
    "setout", &cmd_setOut,
    "scan", &cmd_scan,

};
#define CMD_DICT_SIZE (sizeof(CMDdict)/sizeof(struct CMD_dictionary))
static void cmd_help(uint32_t argc, char *argv[])
{
    for(uint8_t idx = 0; idx < CMD_DICT_SIZE; idx++)
    {
        printf("%d. %s\n", idx+1, CMDdict[idx].cmdName);
    }
}

static void cmd_mqttSendMessage(uint32_t argc, char *argv[])
{
    char msg[PUB_MSG_LEN];
    snprintf(msg, PUB_MSG_LEN, argv[1]);
    printf("terminal send message %s\n", msg);
    xQueueSend(publish_queue, msg,0);
}

static void cmd_reboot(uint32_t argc, char *argv[])
{
    printf("Rebooting...\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    sdk_system_restart();
}

static void cmd_printSpineDAta(uint32_t argc, char *argv[])
{
    DATAMANAGER_printSpineData();
}

static void cmd_setSpineData(uint32_t argc, char *argv[])
{
    printf("argc %d\n", argc);
    if(argc < 4)
    {
        printf("Not enough arguments. Try NAME SSID PASSWORD\n");
        return;
    }
    else
    {
        TSpineConfigDataStruct configData;
        strcpy(&configData.name, argv[1]);
        strcpy(&configData.ssid, argv[2]);
        strcpy(&configData.password, argv[3]);
        DATAMANAGER_setSpineData(&configData);
    }
    
}

static void cmd_setOut(uint32_t argc, char *argv[])
{
    
    if(argc < 1)
    {
        printf("Not enough arguments. \n");
        return;
    }
    else
    {
        printf("Set out %d\n", atoi(argv[1]));
        IO_setOut(atoi(argv[1]));
    }
}

static void cmd_scan(uint32_t argc, char *argv[])
{
    WIFI_scanNetwork();
}




static void handle_command(char *cmd)
{
    char *argv[MAX_ARGC];
    int argc = 1;
    char *temp, *rover;
    memset((void*) argv, 0, sizeof(argv));
    argv[0] = cmd;
    rover = cmd;
    // Split string "<command> <argument 1> <argument 2>  ...  <argument N>"
    // into argv, argc style
    while(argc < MAX_ARGC && (temp = strstr(rover, " "))) {
        rover = &(temp[1]);
        argv[argc++] = rover;
        *temp = 0;
    }
    if (strlen(argv[0]) > 0)
    {
        for(uint16_t cmdIdx = 0; cmdIdx < CMD_DICT_SIZE; cmdIdx++)
        {
            if(strcmp(argv[0], CMDdict[cmdIdx].cmdName) == 0) 
            {
                CMDdict[cmdIdx].cmdHandler(argc, argv);
                return;
            }
        }
        printf("Unknown command %s, try 'help'\n", argv[0]);
    }

}

static void terminal_task()
{
    char ch;
    char cmd[81];
    int i = 0;
    printf("\n\n\nWelcome in Spine! module!\n");
    printf("Cmd dict size: %d\n", sizeof(CMDdict)/sizeof(struct CMD_dictionary) );
    printf(PROMPT);
    fflush(stdout); // stdout is line buffered
    while(1) 
    {
        if (read(0, (void*)&ch, 1)) 
        { // 0 is stdin
            //printf("%c", ch);
            //printf("%d",ch);
            
            if (ch == '\n' || ch == '\r')
            {
                cmd[i] = 0;
                i = 0;
                printf("\n");
                handle_command((char*) cmd);
                printf("\n%s",PROMPT);
                
            } 
            else if(ch == '\b' || ch == '\x7f')
            {
                if(i > 0)
                {
                    i--;
                    printf("\b \b");
                }
            }
            else 
            {
                if (i < sizeof(cmd)) 
                    cmd[i++] = ch;
                printf("%c", ch);
            }
            fflush(stdout);
        }
        else 
        {
            printf("You will never see this print as read(...) is blocking\n");
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

int TERMINAL_init(void)
{
    printf("TERMINAL_init()\n");
    if(xTaskCreate(&terminal_task, "terminal_task", 1024, NULL, 3, NULL) == pdPASS)
        return true;
    else
        return false;
}