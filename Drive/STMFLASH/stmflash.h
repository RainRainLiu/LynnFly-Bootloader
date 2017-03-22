#ifndef __STMFLASH_H__
#define __STMFLASH_H__
#include "sys.h"  
#include "config.h"
#define STM32_FLASH_BASE    0x08000000 			//STM32 FLASH����ʼ��ַ

#define STM32_FLASH_WREN 	1              		//ʹ��FLASHд��(0��������;1��ʹ��)




#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif



void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);
void STMFLASH_Read(u32 ReadAddr,uint8_t *pBuffer,u32 NumToRead);   
ErrorStatus STMFLASH_EraseAll(uint32_t padd, uint32_t size);

#endif

