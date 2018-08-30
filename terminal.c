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

#define MAX_ARGC (10)

#define PROMPT "CMD> "

extern QueueHandle_t publish_queue;


static void cmd_help(uint32_t argc, char *argv[])
{
    printf("on <gpio number> [ <gpio number>]+     Set gpio to 1\n");
    printf("off <gpio number> [ <gpio number>]+    Set gpio to 0\n");
    printf("sleep                                  Take a nap\n");
    printf("\nExample:\n");
    printf("  on 0<enter> switches on gpio 0\n");
    printf("  on 0 2 4<enter> switches on gpios 0, 2 and 4\n");
}

static void cmd_mqttSendMessage(uint32_t argc, char *argv[])
{
    char msg[PUB_MSG_LEN];
    snprintf(msg, PUB_MSG_LEN, argv[0]);
    printf("terminal send message %s\n", msg);
    xQueueSend(publish_queue, msg,0);
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

    if (strlen(argv[0]) > 0) {
        if (strcmp(argv[0], "help") == 0) cmd_help(argc, argv);
        else if (strcmp(argv[0], "sl") == 0) cmd_mqttSendMessage(argc, argv);
        else printf("Unknown command %s, try 'help'\n", argv[0]);
    }
}

static void terminal_task()
{
    char ch;
    char cmd[81];
    int i = 0;
    printf("\n\n\nWelcome in Spine! module!\n");
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
                printf(PROMPT);
                
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
    if(xTaskCreate(&terminal_task, "terminal_task", 256, NULL, 3, NULL) == pdPASS)
        return true;
    else
        return false;
}