#ifndef __STMFLASH_H__
#define __STMFLASH_H__

#include "stm32f1xx_hal.h"
#include "config.h"

#define STM32_FLASH_BASE    0x08000000 			//STM32 FLASH的起始地址



#define STM_SECTOR_SIZE 1024 //字节



ErrorStatus STMFLASH_Write(uint32_t nWriteAddr, uint8_t *pData, uint32_t nLength);
ErrorStatus STMFLASH_Erase(uint32_t padd, uint32_t size);
void STMFLASH_Read(uint32_t ReadAddr, uint8_t *pBuffer, uint32_t nLength);

#endif

