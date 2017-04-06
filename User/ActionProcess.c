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
FIRMWARE_INFO_T firmwareInfo;
COMM_DATA_PACK_T packet;
/******************************************************************
  * @函数说明：   处理数据包
  * @输入参数：   COMM_DATA_PACK_T *pPacket 接收的数据包
  * @输出参数：   
  * @返回参数：   无             
  * @修改记录：   ----
******************************************************************/
void Action_SendAck(uint8_t nCMD,uint8_t nState)
{
    packet.nCMD = nCMD;
    packet.nLength = 1;
    packet.aData[0] = nState;
    
    COMM_BuildAndSendPacket(&packet);
}




/******************************************************************
  * @函数说明：   处理数据包
  * @输入参数：   COMM_DATA_PACK_T *pPacket 接收的数据包
  * @输出参数：   
  * @返回参数：   无             
  * @修改记录：   ----
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
                    packet.aData[packet.nLength++] = 0x00;  //状态正常
                }
                else
                {
                    packet.aData[packet.nLength++] = 0;
                    packet.aData[packet.nLength++] = 0x01;  //状态异常
                }
                
            }
            else
            {
                packet.aData[packet.nLength++] = 0;
                packet.aData[packet.nLength++] = 0x01;  //状态异常
            }
            COMM_BuildAndSendPacket(&packet);
        }
        break;           
        case COM_CMD_ERASURE:
        {
            uint8_t result = 0;
            uint32_t size, page;
            
            memcpy(&size, pPacket->aData, 4);
            
            if (size == 0)
            {
                if (FirmwareMange_ReadFirmwareInfo(&firmwareInfo) == SUCCESS)
                {
                    size = firmwareInfo.nLength;
                    result = 0;
                }
                else
                {
                    result = 1;
                }
            }

            Action_SendAck(COM_CMD_ERASURE, result);
            
            if (result == 0)
            {
                page = size / STM_SECTOR_SIZE;
            
                if (size % STM_SECTOR_SIZE)
                {
                    page += 1;
                }
                
                STMFLASH_EraseAll(APP_STATR_ADDRESS, page);
            
                Action_SendAck(COM_CMD_ERASURE_REPORT, 0x01);   //上报擦除完成
            }
        }
        break;        
        case COM_CMD_ERASURE_REPORT:
        {
            
        }
        break; 
        
        case COM_CMD_DOWNLOAD_INFO:
        {
            memcpy(&firmwareInfo.nLength, pPacket->aData, 4);    //长度

            memcpy(&firmwareInfo.nFileCRC, &pPacket->aData[4], 4);    //CRC

            memcpy(&firmwareInfo.nOffsetValue, &pPacket->aData[8], 4);    //偏移地址
            
            firmwareInfo.nVersionLength = pPacket->aData[12];
            
            memcpy(firmwareInfo.aVersion, &pPacket->aData[13], firmwareInfo.nVersionLength);
            
            if (FirmwareMange_WriteFirmwareInfo(&firmwareInfo) == SUCCESS)
            {
                Action_SendAck(COM_CMD_DOWNLOAD_INFO, 0x00);
                firmwareWrite.pFirmwareInfo = & firmwareInfo;
            }
            else
            {
                Action_SendAck(COM_CMD_DOWNLOAD_INFO, 0x01);
            }
        }
        break;

        case COM_CMD_DOWNLOAD_DATA:
        {
            uint32_t packNum;
            uint32_t packID;
            memcpy(&packNum, pPacket->aData, 4);    //长度
            memcpy(&packID, &pPacket->aData[4], 4);
            
            firmwareWrite.nPackNumber = packNum;
            
            if (FirmwareMange_WriteFirmware(&firmwareWrite, packID, &pPacket->aData[8], pPacket->nLength - 8) == SUCCESS)
            {
                Action_SendAck(COM_CMD_DOWNLOAD_DATA, 0x00);
            }
            else
            {
                Action_SendAck(COM_CMD_DOWNLOAD_DATA, 0x01);
            }
        }
        break;
        
        case COM_CMD_RUN_APP:
        {
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




