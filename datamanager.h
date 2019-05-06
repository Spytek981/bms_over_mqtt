#ifndef __DATAMANAGER_H__
#define __DATAMANAGER_H__

    #define SPINE_DATA_FILE_NAME ("spinedata.bin")
    typedef struct
    {
        uint32_t statusRegister;
        uint8_t ssid[32];     /* Null terminated string */
        uint8_t password[64]; /* Null terminated string */
        uint8_t name[32];
        uint8_t wifiMode;     // 0 - ap / 1 - client
    }TSpineConfigDataStruct;

    void DATAMANAGER_init(void);
    void DATAMANAGER_printSpineData(void);
    int8_t DATAMANAGER_setSpineData(TSpineConfigDataStruct *newdata );
    int8_t DATAMANAGER_getSpineData(TSpineConfigDataStruct *dataContainer );
    int8_t DATAMANAGER_saveSpineData(void);
    int8_t DATAMANAGER_readSpineData(void);
    int8_t DATAMANAGER_getWiFiMode(void);



#endif
