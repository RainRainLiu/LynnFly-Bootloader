
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
  * @�������ƣ�   void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
  * @����˵����   ������д��
  * @���������   WriteAddr:��ʼ��ַ
                 pBuffer:����ָ��
                 NumToWrite:����(16λ)��
  * @���������   None   w
  * @���ز�����   ��ȡ��������             
  * @�޸ļ�¼��   ----
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
  * @�������ƣ�   void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead) 
  * @����˵����   ������д��
  * @���������   ReadAddr:��ʼ��ַ
                 pBuffer:����ָ��
                 NumToWrite:����(16λ)��
  * @���������   None
  * @���ز�����   None             
  * @�޸ļ�¼��   ----
******************************************************************/
void STMFLASH_Read(uint32_t ReadAddr, uint8_t *pBuffer, uint32_t nLength)   	
{
	uint32_t i;
	for (i = 0; i < nLength; i++)
	{
		pBuffer[i]=*(uint8_t*)ReadAddr; //STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr++;//ƫ��1���ֽ�.	
	}
}


/******************************************************************
  * @�������ƣ�   ErrorStatus STMFLASH_EraseAll(uint32_t padd, uint32_t flash_size)
  * @����˵����   ����ҳ����
  * @���������   uint32_t padd����ʼ��ַ
                 uint32_t size ��Ҫ�������� K
  * @���������   None
  * @���ز�����   ErrorStatus             
  * @�޸ļ�¼��   ----
******************************************************************/
ErrorStatus STMFLASH_Erase(uint32_t padd, uint32_t size)
{
    //uint32_t end_add = 0;

    uint32_t pagError;
    
    //end_add = padd + STM_SECTOR_SIZE * size;           //������ַ
    


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







