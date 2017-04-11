#include "FirmwareManage.h"
#include "config.h"
#include "string.h"
#include "CRC32.h"
#include "stmflash.h"

typedef struct
{
    uint32_t nCRC;
    FIRMWARE_INFO_T info;
}FIRMWARE_INFO_FLASH_T;


/******************************************************************
  * @����˵����   ���̼���Ϣ�Ƿ���������Χ
  * @���������   FIRMWARE_INFO_T *firmwareInfo �̼���Ϣ
  * @���������   ��
  * @���ز�����   ���ؽ�� sucess error
  * @�޸ļ�¼��   
******************************************************************/
static ErrorStatus checkInfo(FIRMWARE_INFO_T *firmwareInfo)
{
    uint32_t totalSize = firmwareInfo->nLength + firmwareInfo->nOffsetValue;
    
    if (firmwareInfo->nOffsetValue < (BOOTLOAD_SIZE + BOOTLOAD_INFO_SIZE + FIRMWARE_INFO_SIZE) 
            || totalSize > (STM32_FLASH_SIZE * 1024))       //��С������Χ
    {
        return ERROR;
    }
    
    if (firmwareInfo->nVersionLength > FIRMWARE_VERSION_MAX_SIZE)
    {
        return ERROR;
    }
    
    return SUCCESS;
}



/******************************************************************
  * @����˵����   ���̼��Ƿ�У���Ƿ���ȷ
  * @���������   FIRMWARE_INFO_T *firmwareInfo �̼���Ϣ
  * @���������   ��
  * @���ز�����   ���ؽ�� sucess error
  * @�޸ļ�¼��   
******************************************************************/
ErrorStatus FirmwareMange_Check(FIRMWARE_INFO_T *firmwareInfo)
{
    
    if (checkInfo(firmwareInfo) == ERROR)
    {
        return ERROR;
    }
    uint32_t address = STM32_FLASH_BASE + firmwareInfo->nOffsetValue;
    
    uint8_t *firmware = (uint8_t *)address;
    
    volatile static uint32_t crc;
    crc = CRC32(firmware, firmwareInfo->nLength);
    
    if (firmwareInfo->nFileCRC == crc)
    {
        return SUCCESS;
    }
    else
    {
        return ERROR;
    }
}

/******************************************************************
  * @����˵����   ���й̼�
  * @���������   �̼���ַ
  * @���������   ��
  * @���ز�����   wu       
  * @�޸ļ�¼��   
******************************************************************/
void FirmwareMange_RunFirmware(uint32_t appxaddr)
{
    
    void 		(*pApp)(void);
	uint32_t	JumpAddress;
	
	JumpAddress = *(__IO uint32_t*) (appxaddr+ 4);
	pApp = (void (*)(void))JumpAddress;
	

	__set_MSP(*(__IO uint32_t*) appxaddr);
 
	(*pApp)();
}		 




/******************************************************************
  * @����˵����   д��̼���Ϣ
  * @���������   FIRMWARE_INFO_T *firmwareInfo �̼���Ϣ�ṹ
  * @���������   �� 
  * @���ز�����   ���󷵻� -1       
  * @�޸ļ�¼��   
******************************************************************/
ErrorStatus FirmwareMange_WriteFirmwareInfo(FIRMWARE_INFO_T *firmwareInfo)
{
    if (checkInfo(firmwareInfo) == ERROR)
    {
        return ERROR;
    }
    
    FIRMWARE_INFO_FLASH_T infoFlah;
    
    infoFlah.nCRC = CRC32((uint8_t *)firmwareInfo, sizeof(FIRMWARE_INFO_T));
    
    memcpy(&infoFlah.info, firmwareInfo, sizeof(FIRMWARE_INFO_T));
    
    uint32_t length = sizeof(FIRMWARE_INFO_FLASH_T);
    
    
    if (length % 2)
    {
        length += 1;
    }
    STMFLASH_Erase(FIRMWARE_INFO_ADDRESS, 1);   //����     
    
    return STMFLASH_Write(FIRMWARE_INFO_ADDRESS, (uint8_t *)&infoFlah, length);
    
}
/******************************************************************
  * @����˵����   д��̼���Ϣ
  * @���������   FIRMWARE_INFO_T *firmwareInfo �̼���Ϣ�ṹ
  * @���������   �� 
  * @���ز�����   ���󷵻� -1       
  * @�޸ļ�¼��   
******************************************************************/
ErrorStatus FirmwareMange_ReadFirmwareInfo(FIRMWARE_INFO_T *firmwareInfo)
{
    FIRMWARE_INFO_FLASH_T infoFlash;
    
    STMFLASH_Read(FIRMWARE_INFO_ADDRESS, (uint8_t *)&infoFlash, sizeof(FIRMWARE_INFO_FLASH_T));
    
    if (infoFlash.nCRC == CRC32((uint8_t *)&infoFlash.info, sizeof(FIRMWARE_INFO_T)) && 
        checkInfo(&infoFlash.info) == SUCCESS)
    {
        memcpy(firmwareInfo, &infoFlash.info, sizeof(FIRMWARE_INFO_T));
        return SUCCESS;
    }
    else
    {
        return ERROR;
    }
}


/******************************************************************
  * @����˵����   д��̼�
  * @���������   FIRMWARE_WRITE_HANDLE_T *handle д��̼��ľ��
                 uint32_t nPackId,               �����
                 uint8_t *pData,                 ����ָ��
                 uint32_t nLength                ���ݳ���
  * @���������   �� 
  * @���ز�����   ���       
  * @�޸ļ�¼��   
******************************************************************/
ErrorStatus FirmwareMange_WriteFirmware(FIRMWARE_WRITE_HANDLE_T *handle, uint32_t nPackId, uint8_t *pData, uint32_t nLength)
{
    ErrorStatus status;
    
    if (nPackId == 0)
    {
        handle->nAddress = STM32_FLASH_BASE + handle->pFirmwareInfo->nOffsetValue;  //��ʼ��ַ
    }
    else if (handle->nPackId != (nPackId - 1)) //��������
    {
        return ERROR;
    }
    handle->nPackId = nPackId;
    
    if (handle->nPackId == (handle->nPackNumber - 1) && (nLength % 2) > 0) //β�ͣ�����������дһ���ֽ�
    {
        status = STMFLASH_Write(handle->nAddress, (uint8_t *)pData, nLength);
        STMFLASH_Write(handle->nAddress, (uint8_t *)pData, nLength);
        //STMFLASH_Write_NoCheck(handle->nAddress, (uint16_t *)pData, (nLength / 2) + 1);
        handle->nAddress += nLength + 2;
    }
    else if ((nLength % 2) > 0)     //��������������
    {
        status = ERROR;
    }
    else
    {
        status = STMFLASH_Write(handle->nAddress, (uint8_t *)pData, nLength);
        handle->nAddress += nLength;
    }
    
    return status;
}



