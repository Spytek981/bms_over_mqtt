#ifndef __DATAMANAGER_H__
#define __DATAMANAGER_H__

    typedef struct SpineConfigDataStruct
    {
        uint32_t statusRegister;
        uint8_t ssid[32];     /* Null terminated string */
        uint8_t password[64]; /* Null terminated string */
        uint8_t name[32];
    }SpineConfigDataStruct;

    void DATAMANAGER_init(void);
    void DATAMANAGER_printSpineData(void);
    int8_t DATAMANAGER_setSpineData(struct SpineConfigDataStruct *newdata );


#endif
