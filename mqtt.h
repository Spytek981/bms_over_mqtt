#ifndef _MQTT_H_
#define _MQTT_H_
#include <paho_mqtt_c/MQTTESP8266.h>
#include <paho_mqtt_c/MQTTClient.h>

    #define MQTT_HOST ("80.211.230.246")
    #define MQTT_PORT 1883
    #define BASE_TOPIC "/spytek/testy"
    #define MQTT_USER "userNAme"
    #define MQTT_PASS "userPAss"
    #define PUB_MSG_LEN 256

    typedef struct mqttMessageContainer
    {
        mqtt_message_t message;
        char messagePayload[256];
        char messageTopic[128];
    } mqttMessageContainer;

    int MQTT_init(void);
    int mqtt_spine_send_event(int eventValue);
#endif