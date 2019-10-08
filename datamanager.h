#ifndef __DATAMANAGER_H__
#define __DATAMANAGER_H__

    #define SPINE_DATA_FILE_NAME ("spinedata.bin")
    typedef struct
    {
        uint32_t statusRegister;
        uint8_t buildingId[37];
        uint8_t ssid[32];     /* Null terminated string */
        uint8_t password[64]; /* Null terminated string */
        uint8_t name[32];
        uint8_t wifiMode;     // 0 - ap / 1 - client
        uint8_t uid[16];
        uint32_t groups;
        uint8_t event_onClose_topic[16];
        uint8_t event_onClose_msg[16];
        uint8_t event_onOpen_topic[16];
        uint8_t event_onOpen_msg[16];
    }TSpineConfigDataStruct;

    void DATAMANAGER_init(void);
    void DATAMANAGER_printSpineData(void);
    int8_t DATAMANAGER_setSpineData(TSpineConfigDataStruct *newdata );
    int8_t DATAMANAGER_getSpineData(TSpineConfigDataStruct *dataContainer );
    int8_t DATAMANAGER_saveSpineData(void);
    int8_t DATAMANAGER_readSpineData(void);
    int8_t DATAMANAGER_getWiFiMode(void);



#endif
