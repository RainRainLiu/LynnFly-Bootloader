
/******************************************************************************
* @ File name --> stmflash.h
* @ Author    --> By@ liuyu
* @ Version   --> V1.0
* @ Date      --> 030 - 12 - 2015
* @ Brief     --> 
*
* @ Copyright (C) 20**
* @ All rights reserved
*******************************************************************************
*
*                                  File Update
* @ Version   --> V1.1
* @ Author    --> 
* @ Date      --> 
* @ Revise    --> 
* @           --> 
*

*
******************************************************************************/
#include "stmflash.h"
#include "stm32f1xx_hal.h"


/******************************************************************
  * @函数名称：   void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
  * @函数说明：   不检查的写入
  * @输入参数：   WriteAddr:起始地址
                 pBuffer:数据指针
                 NumToWrite:半字(16位)数
  * @输出参数：   None   w
  * @返回参数：   读取到的数据             
  * @修改记录：   ----
******************************************************************/
ErrorStatus STMFLASH_Write(uint32_t nWriteAddr, uint8_t *pData, uint32_t nLength)   
{ 			 		 
	uint32_t i;
    HAL_FLASH_Unlock();
    ErrorStatus statue = SUCCESS;
    
    for (i = 0; i < nLength; i += 2)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, nWriteAddr + i, *(uint32_t*)pData) != HAL_OK)
        {
            statue = ERROR;
            break;
        }
        if (*(uint16_t *)pData != *(uint16_t*)(nWriteAddr + i))
        {
            statue = ERROR;
            break;
        }
        pData += 2;
    }
    HAL_FLASH_Lock();
    
    return statue;
} 



/******************************************************************
  * @函数名称：   void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead) 
  * @函数说明：   不检查的写入
  * @输入参数：   ReadAddr:起始地址
                 pBuffer:数据指针
                 NumToWrite:半字(16位)数
  * @输出参数：   None
  * @返回参数：   None             
  * @修改记录：   ----
******************************************************************/
void STMFLASH_Read(uint32_t ReadAddr, uint8_t *pBuffer, uint32_t nLength)   	
{
	uint32_t i;
	for (i = 0; i < nLength; i++)
	{
		pBuffer[i]=*(uint8_t*)ReadAddr; //STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr++;//偏移1个字节.	
	}
}


/******************************************************************
  * @函数名称：   ErrorStatus STMFLASH_EraseAll(uint32_t padd, uint32_t flash_size)
  * @函数说明：   连续页擦除
  * @输入参数：   uint32_t padd：起始地址
                 uint32_t size ：要擦除容量 K
  * @输出参数：   None
  * @返回参数：   ErrorStatus             
  * @修改记录：   ----
******************************************************************/
ErrorStatus STMFLASH_Erase(uint32_t padd, uint32_t size)
{
    //uint32_t end_add = 0;

    uint32_t pagError;
    
    //end_add = padd + STM_SECTOR_SIZE * size;           //结束地址
    


    HAL_FLASH_Unlock();
    static FLASH_EraseInitTypeDef eraseEx;
    
    eraseEx.Banks = FLASH_BANK_1;
    eraseEx.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseEx.NbPages = size;
    eraseEx.PageAddress = padd;
    
    if (HAL_FLASHEx_Erase(&eraseEx, &pagError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return ERROR;
    }

    
    //for(; padd<=end_add; padd+=STM_SECTOR_SIZE)
    //{
    //    FLASH_PageErase(padd);
    //    if (FLASH_WaitForLastOperation(0xff) != HAL_OK)
    //    {
    //        return ERROR;
    //    }
    //
    //}
    HAL_FLASH_Lock();
    return SUCCESS;
}







