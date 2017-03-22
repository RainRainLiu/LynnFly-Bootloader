
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
  * @函数名称：   void STMFLASH_Unlock(void)
  * @函数说明：   Flash unlock
  * @输入参数：   None
  * @输出参数：   None   
  * @返回参数：   None              
  * @修改记录：   ----
******************************************************************/
void STMFLASH_Unlock(void)
{
    
    FLASH->KEYR=FLASH_KEY1;//写入解锁序列.
    FLASH->KEYR=FLASH_KEY2;
    __NOP();
    __NOP();
    __NOP();
}      
/******************************************************************
  * @函数名称：   void STMFLASH_Lock(void)
  * @函数说明：   Flash lock
  * @输入参数：   None
  * @输出参数：   None   
  * @返回参数：   None              
  * @修改记录：   ----
******************************************************************/
void STMFLASH_Lock(void)
{
  FLASH->CR|=1<<7;  //上锁
}
/******************************************************************
  * @函数名称：   enum enum_FlashStatus STMFLASH_GetStatus(void)
  * @函数说明：   get flash status
  * @输入参数：   None
  * @输出参数：   None   
  * @返回参数：   enum enum_FlashStatus   flash status              
  * @修改记录：   ----
******************************************************************/
enum_FlashStatus STMFLASH_GetStatus(void)
{	
	u32 res;		
	res = FLASH->SR; 
	if(res & (1<<0))
    {
        return Flash_Busy;		    //忙
    }  
	else if(res&(1<<2))
    {
        return Flash_ProgramError;	//编程错误
    }
	else if(res&(1<<4))
    {
        return Flash_WriteLockError;	//写保护错误
    }
	return Flash_Free;						//操作完成
}
/******************************************************************
  * @函数名称：   enum enum_FlashStatus STMFLASH_GetStatus(void)
  * @函数说明：   get flash status
  * @输入参数：   None
  * @输出参数：   None   
  * @返回参数：   enum enum_FlashStatus   flash status              
  * @修改记录：   ----
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
  * @函数名称：   enum enum_FlashStatus STMFLASH_ErasePage(u32 paddr)
  * @函数说明：   get flash status
  * @输入参数：   None
  * @输出参数：   None   
  * @返回参数：   enum enum_FlashStatus   flash status              
  * @修改记录：   ----
******************************************************************/
enum_FlashStatus STMFLASH_ErasePage(u32 paddr)
{
	enum_FlashStatus res;
    
	res = STMFLASH_WaitDone(0X5FFF);//等待上次操作结束,>20ms 
    
	if(res == Flash_Free)
	{ 
		FLASH->CR |= 1<<1;          //页擦除
		FLASH->AR  = paddr;         //设置页地址 
		FLASH->CR |= 1<<6;          //开始擦除
        
		res = STMFLASH_WaitDone(0X5FFF);//等待操作结束,>20ms
        
		if(res != Flash_Busy)   //非忙
		{
			FLASH->CR&=~(1<<1); //清除页擦除标志.
		}
	}
	return res;
}

/******************************************************************
  * @函数名称：   enum enum_FlashStatus STMFLASH_ErasePage(u32 paddr)
  * @函数说明：   在FLASH指定地址写入半字
  * @输入参数：   faddr:指定地址(此地址必须为2的倍数!!)
                 dat:要写入的数据
  * @输出参数：   None   
  * @返回参数：   enum enum_FlashStatus   flash status              
  * @修改记录：   ----
******************************************************************/
enum_FlashStatus STMFLASH_WriteHalfWord(u32 faddr, u16 dat)
{
	enum_FlashStatus res;
    
	res = STMFLASH_WaitDone(0XFF);
    
	if(res == Flash_Free)//OK
	{
		FLASH->CR |= 1<<0;//编程使能
		*(vu16*)faddr = dat;//写入数据
		res = STMFLASH_WaitDone(0XFF);//等待操作完成
		if(res != Flash_Free)//操作成功
		{
			FLASH->CR&=~(1<<0);//清除PG位.
		}
        if (*(vu16*)faddr != dat)
        {
            return Flash_ProgramError;
        }
	} 
	return res;
} 

/******************************************************************
  * @函数名称：   u16 STMFLASH_ReadHalfWord(u32 faddr)
  * @函数说明：   读取指定地址的半字(16位数据)
  * @输入参数：   faddr:读地址 
  * @输出参数：   None   
  * @返回参数：   读取到的数据             
  * @修改记录：   ----
******************************************************************/
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}  

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
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		STMFLASH_WriteHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//地址增加2.
	}  
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
void STMFLASH_Read(u32 ReadAddr,uint8_t *pBuffer,u32 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=*(vu8*)ReadAddr; //STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
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
ErrorStatus STMFLASH_EraseAll(uint32_t padd, uint32_t size)
{
    uint32_t end_add = 0;
    enum_FlashStatus res;
    
    end_add = padd+1024 * size;           //结束地址
    


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
  * @函数名称：   void STMFLASH_Write_Char(u32 WriteAddr,u8 *pBuffer,u16 NumToWrite)
  * @函数说明：   字节写入
  * @输入参数：   WriteAddr:起始地址
                 pBuffer:数据指针
                 NumToWrite:半字(16位)数
  * @输出参数：   None   
  * @返回参数：              
  * @修改记录：   ----
******************************************************************/
ErrorStatus STMFLASH_Write_Char(u32 WriteAddr,u8 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
    u16 half_word;
    u16 temp;
    
    if(WriteAddr<STM32_FLASH_BASE ||(WriteAddr + NumToWrite)>(STM32_FLASH_BASE+1024 * STM32_FLASH_SIZE))
    {
        return ERROR;//非法地址
    }
	for(i=0;i<NumToWrite;i+=2)
	{
        temp = pBuffer[i] << 8;
        half_word = temp | pBuffer[i+1];
		STMFLASH_WriteHalfWord(WriteAddr,half_word);
	    WriteAddr+=2;//地址增加2.
    }
    return SUCCESS;
}








