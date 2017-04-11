#include "ActionProcess.h"
#include "string.h"
#include "FirmwareManage.h"
#include "Communcation.h"
#include "usart.h"

#define COM_CMD_HEARBEAT        0X01
#define COM_CMD_INFO            0X02
#define COM_CMD_ERASURE         0X03
#define COM_CMD_ERASURE_REPORT  0X04
#define COM_CMD_DOWNLOAD_INFO   0X05
#define COM_CMD_DOWNLOAD_DATA   0X06
#define COM_CMD_RUN_APP         0X07


typedef enum
{
    ACK_SUCESS = 0,
    ACK_ERROR = 1,
    
}COM_ACK_RESULT_T;


FIRMWARE_WRITE_HANDLE_T firmwareWrite;
FIRMWARE_INFO_T firmwareInfo;
COMM_DATA_PACK_T packet;
uint32_t waiteTime;
uint8_t runFlag = 0;
/******************************************************************
  * @����˵����   �������ݰ�
  * @���������   COMM_DATA_PACK_T *pPacket ���յ����ݰ�
  * @���������   
  * @���ز�����   ��             
  * @�޸ļ�¼��   ----
******************************************************************/
static void Action_SendAck(uint8_t nCMD,uint8_t nState)
{
    packet.nCMD = nCMD;
    packet.nLength = 1;
    packet.aData[0] = nState;
    
    COMM_BuildAndSendPacket(&packet);
}


static void Action_AutoRun(void)
{
    if (runFlag == 0)
    {
        if (FirmwareMange_ReadFirmwareInfo(&firmwareInfo) == SUCCESS)
        {
            if (FirmwareMange_Check(&firmwareInfo) == SUCCESS)
            {
                runFlag = 1;
                waiteTime = 0xfffff;
                return;
            }
        }
        runFlag = 2; //���Զ�����
    }
    else if (runFlag == 1)
    {
        waiteTime--;
        if (waiteTime == 0)
        {
             FirmwareMange_RunFirmware(firmwareInfo.nOffsetValue);
        }
    }
}


/******************************************************************
  * @����˵����   �������ݰ�
  * @���������   COMM_DATA_PACK_T *pPacket ���յ����ݰ�
  * @���������   
  * @���ز�����   ��             
  * @�޸ļ�¼��   ----
******************************************************************/

static void Action_PacketProcess(COMM_DATA_PACK_T *pPacket)
{
    runFlag = 2;
    switch(pPacket->nCMD)
    {
        case COM_CMD_HEARBEAT:
        {
            Action_SendAck(COM_CMD_HEARBEAT, ACK_SUCESS);
        }
        break;
        case COM_CMD_INFO:
        {
            
            packet.nCMD = COM_CMD_INFO;
            packet.aData[0] = sizeof(BOOTLOAD_VERSION);
            memcpy(&packet.aData[1], BOOTLOAD_VERSION, packet.aData[0]);
            packet.nLength = packet.aData[0] + 1;
            
            if (FirmwareMange_ReadFirmwareInfo(&firmwareInfo) == SUCCESS)
            {
                if (FirmwareMange_Check(&firmwareInfo) == SUCCESS)
                {
                    packet.aData[packet.nLength++] = firmwareInfo.nVersionLength;
                    memcpy(&packet.aData[packet.nLength], firmwareInfo.aVersion, firmwareInfo.nVersionLength);
                    packet.nLength += firmwareInfo.nVersionLength;
                    packet.aData[packet.nLength++] = ACK_SUCESS;  //״̬����
                }
                else
                {
                    packet.aData[packet.nLength++] = 0;
                    packet.aData[packet.nLength++] = ACK_ERROR;  //״̬�쳣
                }
                
            }
            else
            {
                packet.aData[packet.nLength++] = 0;
                packet.aData[packet.nLength++] = ACK_ERROR;  //״̬�쳣
            }
            COMM_BuildAndSendPacket(&packet);
        }
        break;           
        case COM_CMD_ERASURE:
        {
            uint8_t result = 0;
            uint32_t size, page;
            uint32_t startAddress,writeAddress;
            
            memcpy(&size, pPacket->aData, 4);
            memcpy(&startAddress, &pPacket->aData[4], 4);
            
            if (size == 0)
            {
                if (FirmwareMange_ReadFirmwareInfo(&firmwareInfo) == SUCCESS)
                {
                    writeAddress = FLASH_BASE + firmwareInfo.nOffsetValue;
                    size = firmwareInfo.nLength;
                    result = ACK_SUCESS;
                }
                else
                {
                    result = ACK_ERROR;
                }
            }
            else
            {
                writeAddress = FLASH_BASE + startAddress;
            }
            
            if (writeAddress >= (FIRMWARE_INFO_ADDRESS + 400))
            {
                result = ACK_SUCESS;
            }

            Action_SendAck(COM_CMD_ERASURE, ACK_SUCESS);
            
            if (result == ACK_SUCESS)
            {
                page = size / STM_SECTOR_SIZE;
            
                if ((startAddress % STM_SECTOR_SIZE) > 0)   //ͷ����ҳ����ʼ��ַ�����Զ��һҳ
                {
                    page += 1;
                }
                
                if ((page * STM_SECTOR_SIZE + startAddress) < size) //β��ռһҳ
                {
                    page += 1;
                }
                
                if (STMFLASH_Erase(writeAddress, page + 1) == SUCCESS)   //��ʱ���1ҳ��δ�ҵ�����ԭ��
                {
                    Action_SendAck(COM_CMD_ERASURE_REPORT, ACK_SUCESS);   //�ϱ��������
                }
                else
                {
                    Action_SendAck(COM_CMD_ERASURE_REPORT, ACK_ERROR);   //�ϱ��������
                }
            }
        }
        break;        
        case COM_CMD_ERASURE_REPORT:
        {
            
        }
        break; 
        
        case COM_CMD_DOWNLOAD_INFO:
        {
            memcpy(&firmwareInfo.nLength, pPacket->aData, 4);    //����

            memcpy(&firmwareInfo.nFileCRC, &pPacket->aData[4], 4);    //CRC

            memcpy(&firmwareInfo.nOffsetValue, &pPacket->aData[8], 4);    //ƫ�Ƶ�ַ
            
            firmwareInfo.nVersionLength = pPacket->aData[12];
            
            memcpy(firmwareInfo.aVersion, &pPacket->aData[13], firmwareInfo.nVersionLength);
            
            if (FirmwareMange_WriteFirmwareInfo(&firmwareInfo) == SUCCESS)
            {
                Action_SendAck(COM_CMD_DOWNLOAD_INFO, ACK_SUCESS);
                firmwareWrite.pFirmwareInfo = & firmwareInfo;
            }
            else
            {
                Action_SendAck(COM_CMD_DOWNLOAD_INFO, ACK_ERROR);
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
                Action_SendAck(COM_CMD_DOWNLOAD_DATA, ACK_SUCESS);
            }
            else
            {
                Action_SendAck(COM_CMD_DOWNLOAD_DATA, ACK_ERROR);
            }
        }
        break;
        
        case COM_CMD_RUN_APP:
        {
            if (FirmwareMange_ReadFirmwareInfo(&firmwareInfo) == SUCCESS)
            {
                if (FirmwareMange_Check(&firmwareInfo) == SUCCESS)
                {
                    Action_SendAck(COM_CMD_RUN_APP, ACK_SUCESS);
                    FirmwareMange_RunFirmware(firmwareInfo.nOffsetValue);
                }
            }
            Action_SendAck(COM_CMD_RUN_APP, ACK_ERROR);
            
        }
        break;
        
        default:
        {
            
        }
        break;

    }
}

COMM_PARSING_T comParsing;

/******************************************************************
  * @����˵����   �������ݵĻص�����
  * @���������   uint8_t nData ����
  * @���������   ��
  * @���ز�����   ��             
  * @�޸ļ�¼��   ----
******************************************************************/
static void Action_ReceiveDataCB(uint8_t nData)
{
    COMM_Parsing(&comParsing, nData);
}
/******************************************************************
  * @����˵����   �������ݵĻص�����
  * @���������   uint8_t nData ����
  * @���������   ��
  * @���ز�����   ��             
  * @�޸ļ�¼��   ----
******************************************************************/
static void Action_SendDataCB(uint8_t *pData, uint32_t nLength)
{
    HAL_UART_Transmit_IT(&huart1, (uint8_t *)pData, nLength);
}

/******************************************************************
  * @����˵����   loop
  * @���������   ��
  * @���������   ��
  * @���ز�����   ��             
  * @�޸ļ�¼��   ----
******************************************************************/
void Action_Process(void)
{
    Action_AutoRun();
    if(comParsing.receiveState == RX_CarryOut)
    {
        Action_PacketProcess(&comParsing.dataPack);
        comParsing.receiveState = RX_Free;
    }
}

/******************************************************************
  * @����˵����   ��ʼ��������������ںͳ���
  * @���������   ��
  * @���������   ��
  * @���ز�����   ��             
  * @�޸ļ�¼��   ----
******************************************************************/
void Action_Init(void)
{
    HAL_UART_SetReceiveCB(Action_ReceiveDataCB);
    COMM_SetOutDataCB(Action_SendDataCB);
}

