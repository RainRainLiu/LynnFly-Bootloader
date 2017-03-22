#include "ActionProcess.h"
#include "stmflash.h"
#include "string.h"
#include "FirmwareManage.h"

#define COM_CMD_HEARBEAT        0X01
#define COM_CMD_INFO            0X02
#define COM_CMD_ERASURE         0X03
#define COM_CMD_ERASURE_REPORT  0X04
#define COM_CMD_DOWNLOAD_INFO   0X05
#define COM_CMD_DOWNLOAD_DATA   0X06
#define COM_CMD_RUN_APP         0X07

#define APP_STATR_ADDRESS       0X08003600

FIRMWARE_WRITE_HANDLE_T firmwareWrite;
COMM_DATA_PACK_T packet;
/******************************************************************
  * @����˵����   �������ݰ�
  * @���������   COMM_DATA_PACK_T *pPacket ���յ����ݰ�
  * @���������   
  * @���ز�����   ��             
  * @�޸ļ�¼��   ----
******************************************************************/
void Action_SendAck(uint8_t nCMD,uint8_t nState)
{
    packet.nCMD = nCMD;
    packet.nLength = 1;
    packet.aData[0] = nState;
    
    COMM_BuildAndSendPacket(&packet);
}


/******************************************************************
  * @����˵����   �������ݰ�
  * @���������   COMM_DATA_PACK_T *pPacket ���յ����ݰ�
  * @���������   
  * @���ز�����   ��             
  * @�޸ļ�¼��   ----
******************************************************************/

void Action_PacketProcess(COMM_DATA_PACK_T *pPacket)
{
    switch(pPacket->nCMD)
    {
        case COM_CMD_HEARBEAT:
        {
            Action_SendAck(COM_CMD_HEARBEAT, 0x01);
        }
        break;
        case COM_CMD_INFO:
        {
            
        }
        break;           
        case COM_CMD_ERASURE:
        {
            uint32_t size, page;
            
            memcpy(&size, pPacket->aData, 4);
            
            page = size / STM_SECTOR_SIZE;
            
            if (size % STM_SECTOR_SIZE)
            {
                page += 1;
            }

            Action_SendAck(COM_CMD_ERASURE, 0x01);
            
            STMFLASH_EraseAll(APP_STATR_ADDRESS, page);
            
            Action_SendAck(COM_CMD_ERASURE_REPORT, 0x01);   //�ϱ��������
        }
        break;        
        case COM_CMD_ERASURE_REPORT:
        {
            
        }
        break; 
        
        case COM_CMD_DOWNLOAD_INFO:
        {
            FIRMWARE_INFO_T firmwareInfo;
            
            memcpy(&firmwareInfo.nLength, pPacket->aData, 4);    //����

            memcpy(&firmwareInfo.nFileCRC, &pPacket->aData[4], 4);    //CRC

            memcpy(&firmwareInfo.nOffsetValue, &pPacket->aData[8], 4);    //ƫ�Ƶ�ַ
            
            if (FirmwareMange_WriteFirmwareInfo(&firmwareInfo) == SUCCESS)
            {
                Action_SendAck(COM_CMD_DOWNLOAD_INFO, 0x01);
            }
            else
            {
                Action_SendAck(COM_CMD_DOWNLOAD_INFO, 0x00);
            }
        }
        break;

        case COM_CMD_DOWNLOAD_DATA:
        {
            uint32_t packNum;
            uint32_t packID;
            memcpy(&packNum, pPacket->aData, 4);    //����
            memcpy(&packID, &pPacket->aData[4], 4);
            
            firmwareWrite.nPackNumber = packNum;
            
            if (FirmwareMange_WriteFirmware(&firmwareWrite, packID, &pPacket->aData[8], pPacket->nLength - 8) == SUCCESS)
            {
                Action_SendAck(COM_CMD_DOWNLOAD_DATA, 0x01);
            }
            else
            {
                Action_SendAck(COM_CMD_DOWNLOAD_DATA, 0x00);
            }
        }
        break;
        
        case COM_CMD_RUN_APP:
        {
            FIRMWARE_INFO_T firmwareInfo;
            
            
            if (FirmwareMange_ReadFirmwareInfo(&firmwareInfo) == SUCCESS)
            {
                if (FirmwareMange_Check(&firmwareInfo) == SUCCESS)
                {
                    Action_SendAck(COM_CMD_RUN_APP, 0x01);
                    FirmwareMange_RunFirmware(firmwareInfo.nOffsetValue);
                }
            }
            Action_SendAck(COM_CMD_RUN_APP, 0x00);
            
        }
        break;
        
        default:
        {
            
        }
        break;

    }
}




