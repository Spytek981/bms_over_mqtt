#ifndef _MQTT_H_
#define _MQTT_H_

    #define MQTT_HOST ("iot.eclipse.org")
    #define MQTT_PORT 1883
    #define BASE_TOPIC "/spytek/testy/"
    #define MQTT_USER NULL
    #define MQTT_PASS NULL

    #define PUB_MSG_LEN 16

    int MQTT_init(void);

#endif