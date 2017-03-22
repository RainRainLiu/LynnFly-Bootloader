
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
#include "usart.h"

#define FLASH_KEY1               0X45670123
#define FLASH_KEY2               0XCDEF89AB

typedef enum                //Flash status
{
    Flash_Free              = 0,
    Flash_Busy              = 1,
    Flash_ProgramError      = 2,
    Flash_WriteLockError    = 3,
    Flash_TimeOut           = 4
}enum_FlashStatus;

/******************************************************************
  * @�������ƣ�   void STMFLASH_Unlock(void)
  * @����˵����   Flash unlock
  * @���������   None
  * @���������   None   
  * @���ز�����   None              
  * @�޸ļ�¼��   ----
******************************************************************/
void STMFLASH_Unlock(void)
{
    
    FLASH->KEYR=FLASH_KEY1;//д���������.
    FLASH->KEYR=FLASH_KEY2;
    __NOP();
    __NOP();
    __NOP();
}      
/******************************************************************
  * @�������ƣ�   void STMFLASH_Lock(void)
  * @����˵����   Flash lock
  * @���������   None
  * @���������   None   
  * @���ز�����   None              
  * @�޸ļ�¼��   ----
******************************************************************/
void STMFLASH_Lock(void)
{
  FLASH->CR|=1<<7;  //����
}
/******************************************************************
  * @�������ƣ�   enum enum_FlashStatus STMFLASH_GetStatus(void)
  * @����˵����   get flash status
  * @���������   None
  * @���������   None   
  * @���ز�����   enum enum_FlashStatus   flash status              
  * @�޸ļ�¼��   ----
******************************************************************/
enum_FlashStatus STMFLASH_GetStatus(void)
{	
	u32 res;		
	res = FLASH->SR; 
	if(res & (1<<0))
    {
        return Flash_Busy;		    //æ
    }  
	else if(res&(1<<2))
    {
        return Flash_ProgramError;	//��̴���
    }
	else if(res&(1<<4))
    {
        return Flash_WriteLockError;	//д��������
    }
	return Flash_Free;						//�������
}
/******************************************************************
  * @�������ƣ�   enum enum_FlashStatus STMFLASH_GetStatus(void)
  * @����˵����   get flash status
  * @���������   None
  * @���������   None   
  * @���ز�����   enum enum_FlashStatus   flash status              
  * @�޸ļ�¼��   ----
******************************************************************/
enum_FlashStatus STMFLASH_WaitDone(u16 time)
{
	enum_FlashStatus status;
    
    
    while(time--)
    {
        status = STMFLASH_GetStatus();
        if(status != Flash_Busy)
        {
            return status;
        }
    }
    return Flash_TimeOut;
}
/******************************************************************
  * @�������ƣ�   enum enum_FlashStatus STMFLASH_ErasePage(u32 paddr)
  * @����˵����   get flash status
  * @���������   None
  * @���������   None   
  * @���ز�����   enum enum_FlashStatus   flash status              
  * @�޸ļ�¼��   ----
******************************************************************/
enum_FlashStatus STMFLASH_ErasePage(u32 paddr)
{
	enum_FlashStatus res;
    
	res = STMFLASH_WaitDone(0X5FFF);//�ȴ��ϴβ�������,>20ms 
    
	if(res == Flash_Free)
	{ 
		FLASH->CR |= 1<<1;          //ҳ����
		FLASH->AR  = paddr;         //����ҳ��ַ 
		FLASH->CR |= 1<<6;          //��ʼ����
        
		res = STMFLASH_WaitDone(0X5FFF);//�ȴ���������,>20ms
        
		if(res != Flash_Busy)   //��æ
		{
			FLASH->CR&=~(1<<1); //���ҳ������־.
		}
	}
	return res;
}

/******************************************************************
  * @�������ƣ�   enum enum_FlashStatus STMFLASH_ErasePage(u32 paddr)
  * @����˵����   ��FLASHָ����ַд�����
  * @���������   faddr:ָ����ַ(�˵�ַ����Ϊ2�ı���!!)
                 dat:Ҫд�������
  * @���������   None   
  * @���ز�����   enum enum_FlashStatus   flash status              
  * @�޸ļ�¼��   ----
******************************************************************/
enum_FlashStatus STMFLASH_WriteHalfWord(u32 faddr, u16 dat)
{
	enum_FlashStatus res;
    
	res = STMFLASH_WaitDone(0XFF);
    
	if(res == Flash_Free)//OK
	{
		FLASH->CR |= 1<<0;//���ʹ��
		*(vu16*)faddr = dat;//д������
		res = STMFLASH_WaitDone(0XFF);//�ȴ��������
		if(res != Flash_Free)//�����ɹ�
		{
			FLASH->CR&=~(1<<0);//���PGλ.
		}
        if (*(vu16*)faddr != dat)
        {
            return Flash_ProgramError;
        }
	} 
	return res;
} 

/******************************************************************
  * @�������ƣ�   u16 STMFLASH_ReadHalfWord(u32 faddr)
  * @����˵����   ��ȡָ����ַ�İ���(16λ����)
  * @���������   faddr:����ַ 
  * @���������   None   
  * @���ز�����   ��ȡ��������             
  * @�޸ļ�¼��   ----
******************************************************************/
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}  

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
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		STMFLASH_WriteHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	}  
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
void STMFLASH_Read(u32 ReadAddr,uint8_t *pBuffer,u32 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=*(vu8*)ReadAddr; //STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
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
ErrorStatus STMFLASH_EraseAll(uint32_t padd, uint32_t size)
{
    uint32_t end_add = 0;
    enum_FlashStatus res;
    
    end_add = padd+1024 * size;           //������ַ
    


    STMFLASH_Unlock();
    
    for(; padd<=end_add; padd+=STM_SECTOR_SIZE)
    {
        res = STMFLASH_ErasePage(padd);
        if(res != Flash_Free)
        {
            return ERROR;
        }
    }
    
    return SUCCESS;
    
}

/******************************************************************
  * @�������ƣ�   void STMFLASH_Write_Char(u32 WriteAddr,u8 *pBuffer,u16 NumToWrite)
  * @����˵����   �ֽ�д��
  * @���������   WriteAddr:��ʼ��ַ
                 pBuffer:����ָ��
                 NumToWrite:����(16λ)��
  * @���������   None   
  * @���ز�����              
  * @�޸ļ�¼��   ----
******************************************************************/
ErrorStatus STMFLASH_Write_Char(u32 WriteAddr,u8 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
    u16 half_word;
    u16 temp;
    
    if(WriteAddr<STM32_FLASH_BASE ||(WriteAddr + NumToWrite)>(STM32_FLASH_BASE+1024 * STM32_FLASH_SIZE))
    {
        return ERROR;//�Ƿ���ַ
    }
	for(i=0;i<NumToWrite;i+=2)
	{
        temp = pBuffer[i] << 8;
        half_word = temp | pBuffer[i+1];
		STMFLASH_WriteHalfWord(WriteAddr,half_word);
	    WriteAddr+=2;//��ַ����2.
    }
    return SUCCESS;
}








