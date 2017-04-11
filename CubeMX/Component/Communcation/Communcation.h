#ifndef _COMMUN_
#define _COMMUN_
#include "stm32f1xx_hal.h"

#define COMM_MAX_PACK_DATA_LENGTH   1200    


typedef enum{
    RX_Free     = 0,
    RX_CarryOut = 1,
    RX_Busy     = 2,
    RX_Error    = 3
}ReceiveStatus;


typedef struct
{
    uint8_t nCMD;
    uint8_t aData[COMM_MAX_PACK_DATA_LENGTH];
    uint32_t nLength;
    
}COMM_DATA_PACK_T;


typedef struct
{
/*******************************
          private    
********************************/     
    uint8_t     nStep;
    uint32_t    nCount;
    uint8_t     nCheckSum;
    uint8_t     nLastData;
    
/*******************************
          Open    
********************************/ 

    uint8_t nErrorNumber;
    COMM_DATA_PACK_T dataPack;
    ReceiveStatus receiveState;
    
}COMM_PARSING_T;



typedef void (*COMM_DATA_OUT_FUN)(uint8_t * pData, uint32_t nLength);

void COMM_SetOutDataCB(COMM_DATA_OUT_FUN cbFun);
void COMM_Parsing(COMM_PARSING_T *pParsing, uint8_t inData);
void COMM_BuildAndSendPacket(COMM_DATA_PACK_T *pPack);
#endif

