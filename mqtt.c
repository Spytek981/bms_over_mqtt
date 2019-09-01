#include "espressif/esp_common.h"
#include <paho_mqtt_c/MQTTESP8266.h>
#include <paho_mqtt_c/MQTTClient.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>
#include "io.h"
#include "wifi.h"
#include "mqtt.h"
#include <string.h>
#include "utils.h"
#include "datamanager.h"
#define buildingID "91addab8-c059-11e9-9cb5-2a2ae2dbcce4"

mqtt_client_t client = mqtt_client_default;

extern SemaphoreHandle_t wifi_alive;
extern QueueHandle_t publish_queue;
extern EventGroupHandle_t event_bus;
static struct CMD_dictionary 
{
    uint8_t cmdName[16];
    void (*cmdHandler)(uint32_t argc, char*argv[]);
};
static void handle_command(char *cmd);

static void topic_received(mqtt_message_data_t *md)
{
    int i;
    
    mqtt_message_t *message = md->message;
    printf(md->message->payload);
    handle_command(md->message->payload);
}

static void group_cmd_parser(mqtt_message_data_t *md)
{
    printf("group_cmd_parser\n");
    // int i;
    TSpineConfigDataStruct spineData;
    DATAMANAGER_getSpineData(&spineData);
    char* grNoStr = strstr(md->topic->lenstring.data, "group/") + 6;
    // printf("Command for groupNo %s\n", grNoStr);
    if(grNoStr)
    {
        int groupNo = 0;
        sscanf(grNoStr, "%2d", &groupNo);
        printf("Command for groupNo %02d\n", groupNo);
        if(1<<(groupNo-1) & spineData.groups)
        {
            mqtt_message_t *message = md->message;
            handle_command(md->message->payload);
        }
        else
        {
            printf("Not for me!\n");
        }
        
    }
    else
    {
        printf("No group in topic: %s\n", md->topic->cstring);
    }
    
}

static void mqtt_task(void *pvParameters)
{
    int ret = 0;
    struct mqtt_network network;
    char mqtt_client_id[20];
    uint8_t mqtt_buf[512];
    uint8_t mqtt_readbuf[512];
    mqtt_packet_connect_data_t data = mqtt_packet_connect_data_initializer;

    mqtt_network_new(&network);
    memset(mqtt_client_id, 0, sizeof(mqtt_client_id));
    strcpy(mqtt_client_id, "ESP-SPYTOROWE");
    // .strcat(mqtt_client_id, get_my_id());

    while (1)
    {
        printf("MQTT taking semaphore\n");
        xSemaphoreTake(wifi_alive, portMAX_DELAY);
        printf("MqTT taking semaphore - SUCCES\n");
        printf("%s: started\n\r", __func__);
        printf("%s: (Re)connecting to MQTT server %s ... ", __func__,
               MQTT_HOST);
        ret = mqtt_network_connect(&network, MQTT_HOST, MQTT_PORT);
        if (ret)
        {
            printf("mqtt_network_connect error: %d\n\r", ret);
            taskYIELD();
            continue;
        }
        printf("done\n\r");
        mqtt_client_new(&client, &network, 5000, mqtt_buf, 512,
                        mqtt_readbuf, 512);

        data.willFlag = 0;
        data.MQTTVersion = 3;
        data.clientID.cstring = mqtt_client_id;
        data.username.cstring = MQTT_USER;
        data.password.cstring = MQTT_PASS;
        data.keepAliveInterval = 10;
        data.cleansession = 0;
        printf("Send MQTT connect ... ");
        ret = mqtt_connect(&client, &data);
        if (ret)
        {
            printf("mqtt_connect error: %d\n\r", ret);
            mqtt_network_disconnect(&network);
            taskYIELD();
            continue;
        }
        static char cliTopic[128];
        TSpineConfigDataStruct spineData;
        DATAMANAGER_getSpineData(&spineData);
        sprintf(cliTopic, "%s/%s/%s/cmd/", BASE_TOPIC, buildingID, get_my_id());
        printf("Subscribing to CLI topic '%s' ", cliTopic);
        if (mqtt_subscribe(&client, cliTopic, MQTT_QOS1, topic_received) != MQTT_SUCCESS)
        {
            printf("FAILED!!!!\n");
            taskYIELD();
            continue;
        }
        else
        {
            printf("SUCCESS!\n");
        }
        // sprintf(cliTopic, "%s/%s/group/#", BASE_TOPIC, buildingID);
        printf("Subscribing to GROUP topic '%s' ", cliTopic);
        if (mqtt_subscribe(&client, BASE_TOPIC"/"buildingID"/group/#", MQTT_QOS1, group_cmd_parser) != MQTT_SUCCESS)
        {
            printf("FAILED!!!!\n");
            taskYIELD();
            continue;
        }
        else
        {
            printf("SUCCESS!\n");
        }

        

        xQueueReset(publish_queue);

        while (1)
        {

            // char msg[PUB_MSG_LEN - 1] = "\0";
            const mqttMessageContainer msg;
            // memset(msg, 0, PUB_MSG_LEN);
            while (xQueueReceive(publish_queue, (void *)&msg, 0) ==
                   pdTRUE)
            {
                printf("GOT message to publish - topic %s\r\n", msg.messageTopic);
                mqtt_message_t message;
                message.payload = msg.messagePayload;
                message.payloadlen = strlen(msg.messagePayload);
                message.dup = 0;
                message.qos = MQTT_QOS1;
                message.retained = 0;
                // ret = mqtt_publish(&client, BASE_TOPIC, &message);
                ret = mqtt_publish(&client, msg.messageTopic, &message);
                printf("published msg: %s, payloadLen %d\r\n", message.payload, message.payloadlen);
                if (ret != MQTT_SUCCESS)
                {
                    printf("error while publishing message: %d\n", ret);
                    break;
                }
            }

            ret = mqtt_yield(&client, 100);
            if (ret == MQTT_DISCONNECTED)
                break;
        }
        printf("Connection dropped, request restart\n\r");
        mqtt_network_disconnect(&network);
        taskYIELD();
    }
}

int MQTT_init(void)
{
    printf("MQTT_init()\n");
    if (xTaskCreate(&mqtt_task, "mqtt_task", 4096, NULL, 2, NULL) == pdPASS)
        return true;
    else
        return false;
}

int mqtt_spine_send_event_ack(mqtt_message_data_t *md)
{
    printf("%s", __func__);
    xEventGroupSetBits(event_bus, 0x1);
    printf(md->message);
}



int mqtt_spine_send_event(int eventValue)
{
    mqttMessageContainer message;
    int msgID = rand();
    
    sprintf(message.messageTopic, "%s/%s/%s/", BASE_TOPIC, buildingID, get_my_id());
    //sprintf(message.messageTopic, "%s/", BASE_TOPIC);
    char ackTopic[128];
    sprintf(ackTopic, "%s%ld", message.messageTopic, msgID);
    sprintf(message.messagePayload, "{\"msgID\":%ld,\"ack_topic\":\"%s\",\"event\":%d}", msgID, ackTopic, eventValue);
    
    printf("Subscribing to ACK topic '%s' ", ackTopic);
    if (mqtt_subscribe(&client, ackTopic, MQTT_QOS1, mqtt_spine_send_event_ack) != MQTT_SUCCESS)
    {
        printf("FAILED!!!!\n");
    }
    else
    {
        printf("SUCCESS!\n");
    }
    
    xQueueSend(publish_queue, &message, 100);

    switch (xEventGroupWaitBits(event_bus, 0x01, pdTRUE, pdFALSE, 2000/portTICK_PERIOD_MS))
    {
        case 0x01:
        {
            printf("ACK came!\n");
            break;
        }
        default:
        {
            printf("ACK timeout!\n");
            break;
        }
    }
    printf("Usubscribing to ACK topic '%s' ", ackTopic);
    if (mqtt_unsubscribe(&client, ackTopic) != MQTT_SUCCESS)
    {
        printf("FAILED!!!!\n");
    }
    else
    {
        printf("SUCCESS!\n");
    }
    return 0;
}

void mqtt_cmd_answer(char *msg_answer)
{
    mqttMessageContainer message;
    
    sprintf(message.messageTopic, "%s/%s/%s/ans/", BASE_TOPIC, buildingID, get_my_id());
    sprintf(message.messagePayload, "%s", msg_answer);
    xQueueSend(publish_queue, &message, 100);
}

//============== MQTT COMMAND INTERFACE
static void cmd_setout(uint32_t argc, char *argv[]);
static void cmd_resetout(uint32_t argc, char *argv[]);

static void cmd_resetout(uint32_t argc, char *argv[]);
static void cmd_getconfig(uint32_t argc, char *argv[]);
static void cmd_setconfig(uint32_t argc, char *argv[]);



static struct CMD_dictionary CMDdict[]=
{
    "SETOUT", cmd_setout,
    "RESETOUT", cmd_resetout,
    "GETCFG", cmd_getconfig,
    "SETCFG", cmd_setconfig,
};
#define CMD_DICT_SIZE (sizeof(CMDdict)/sizeof(struct CMD_dictionary))



static void cmd_setout(uint32_t argc, char *argv[])
{
    // if(argc != 2)
    // {
    //     printf("%s - ERROR - no argument\n");
    //     return;
    // }
    // else
    // {
        IO_setOut(1);
        mqtt_cmd_answer("Yeah!");
    // }
}

static void cmd_resetout(uint32_t argc, char *argv[])
{
    // if(argc != 2)
    // {
    //     printf("%s - ERROR - no argument\n");
    //     return;
    // }
    // else
    // {
        IO_setOut(0);
        mqtt_cmd_answer("Yeah!");
    // }
}

static void cmd_getconfig(uint32_t argc, char *argv[])
{
    TSpineConfigDataStruct actualConfig;
    DATAMANAGER_getSpineData(&actualConfig);
    
    char ActualConfigStr[256];
    sprintf(
        ActualConfigStr, 
        "{\"statusRegister\":%u,\"ssid\":\"%s\",\"password\":\"%s\",\"name\":\"%s\",\"wifiMode\":%d}",
        actualConfig.statusRegister,
        actualConfig.ssid,
        actualConfig.password,
        actualConfig.name,
        actualConfig.wifiMode
    );

    mqtt_cmd_answer(ActualConfigStr);

}

static void cmd_setconfig(uint32_t argc, char *argv[])
{
    printf("cmd_setconfig\n");
    
    
    if(argc != 11)
    {
        printf("bad arguments\n");
        mqtt_cmd_answer("ERROR - > Bad arguments");
        return;
    }
    TSpineConfigDataStruct newConcig;
    sscanf(argv[1], "%d", &newConcig.statusRegister);
    sscanf(argv[2], "%s", &newConcig.ssid);
    sscanf(argv[3], "%s", &newConcig.password);
    sscanf(argv[4], "%s", &newConcig.name);
    sscanf(argv[5], "%d", &newConcig.wifiMode);
    printf("%s\n",argv[7]);
    sscanf(argv[6], "%x", &newConcig.groups);
    
    printf("%s\n",argv[8]);
    sscanf(argv[7], "%s", newConcig.event_onClose_topic);
    
    sscanf(argv[8], "%s", &newConcig.event_onClose_msg);
    sscanf(argv[9], "%s", &newConcig.event_onOpen_topic);
    sscanf(argv[10], "%s", &newConcig.event_onOpen_msg);
    DATAMANAGER_setSpineData(&newConcig);
    mqtt_cmd_answer("OK");

}

static char* getptrtofirsttag(char * text)
{
    char *temp, *retVal = 0;
    char tagArray[] = " {}\":,";
    for(char *tag = tagArray; *tag != '\0'; tag++)
    {
        temp = strstr(text, tag);
        if(temp)
        {
            if(!retVal || temp < retVal)
            {
                retVal = temp;
            }
        }
    }
    return retVal;

}


#define MAX_ARGC 100
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
    while(argc < MAX_ARGC && (  (temp = strstr(rover, " "))))
    // while(argc < MAX_ARGC && (temp = getptrtofirsttag(rover)))
    {
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
        printf("MQTT CMD -> UNKNOWN COMMAND -> \"%s\"\n", argv[0]);
    }

}