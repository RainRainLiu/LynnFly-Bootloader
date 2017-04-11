#ifndef __FIRMWARE_MANAGE_H__
#define __FIRMWARE_MANAGE_H__
#include "stm32f1xx_hal.h"
#include "stmflash.h"
#include "config.h"

#define FIRMWARE_VERSION_MAX_SIZE   100
#define FIRMWARE_INFO_SIZE          0X400
#define FIRMWARE_INFO_ADDRESS       STM32_FLASH_BASE + BOOTLOAD_SIZE + BOOTLOAD_INFO_SIZE

typedef struct
{
    uint32_t nFileCRC;
    uint32_t nLength;
    uint32_t nOffsetValue;
    uint8_t  nVersionLength;
    uint8_t  aVersion[FIRMWARE_VERSION_MAX_SIZE];
}FIRMWARE_INFO_T;


typedef struct
{
    FIRMWARE_INFO_T *pFirmwareInfo; //固件信息
    uint32_t nAddress;              //当前位置地址
    uint32_t nPackNumber;           //包总数
    uint32_t nPackId;
    
}FIRMWARE_WRITE_HANDLE_T;


ErrorStatus FirmwareMange_WriteFirmware(FIRMWARE_WRITE_HANDLE_T *handle, uint32_t nPackId, uint8_t *pData, uint32_t nLength);
ErrorStatus FirmwareMange_ReadFirmwareInfo(FIRMWARE_INFO_T *firmwareInfo);
ErrorStatus FirmwareMange_WriteFirmwareInfo(FIRMWARE_INFO_T *firmwareInfo);
void FirmwareMange_RunFirmware(uint32_t appxaddr);
ErrorStatus FirmwareMange_Check(FIRMWARE_INFO_T *firmwareInfo);


#endif



