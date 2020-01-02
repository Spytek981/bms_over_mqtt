#ifndef __DATAMANAGER_H__
#define __DATAMANAGER_H__

    #define SPINE_DATA_FILE_NAME ("spinedata.bin")
    typedef struct
    {
        uint8_t name[32];
        uint32_t statusRegister;
        uint8_t buildingId[37];
        uint8_t ssid[32];     /* Null terminated string */
        uint8_t password[64]; /* Null terminated string */
        uint32_t wifiMode;     // 0 - ap / 1 - client
        uint8_t uid[16];
        uint32_t output_groups; //Wyjście może byc sterowane z wielu grup
        unsigned int input_group;    //Wejscie moze sterowac tylko jedną grupą
        unsigned int inputMode;      //Tryb pracy wejścia 0 - monostable, 1 - bistable
    }TSpineConfigDataStruct __attribute__((packed));

    void DATAMANAGER_init(void);
    void DATAMANAGER_printSpineData(void);
    int8_t DATAMANAGER_setSpineData(TSpineConfigDataStruct *newdata );
    int8_t DATAMANAGER_getSpineData(TSpineConfigDataStruct *dataContainer );
    int8_t DATAMANAGER_saveSpineData(void);
    int8_t DATAMANAGER_readSpineData(void);
    int8_t DATAMANAGER_getWiFiMode(void);
    uint32_t DATAMANAGER_getSpineStatus(void);
    void DATAMANAGER_setSpineStatusBit(uint8_t bit, uint8_t value);



#endif
