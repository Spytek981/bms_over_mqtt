#ifndef __DATAMANAGER_H__
#define __DATAMANAGER_H__

    typedef struct
    {
        uint32_t statusRegister;
        uint8_t ssid[32];     /* Null terminated string */
        uint8_t password[64]; /* Null terminated string */
        uint8_t name[32];
    }TSpineConfigDataStruct;

    void DATAMANAGER_init(void);
    void DATAMANAGER_printSpineData(void);
    int8_t DATAMANAGER_setSpineData(TSpineConfigDataStruct *newdata );
    int8_t DATAMANAGER_getSpineData(TSpineConfigDataStruct *dataContainer );
 


#endif
