/******************************************************************************
* @ File name --> Communcation.c
* @ Author    --> By@ LiuYu
* @ Version   --> V1.0
* @ Date      --> 02 - 02 - 2016
* @ Brief     --> Packet parsing
* @           --> 
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
#include "Communcation.h"
#include "stdlib.h"
#include "string.h"
const uint8_t   STX = 0xeb;
const uint8_t   ETX = 0x30;
const uint8_t   ESC = 0xef;


/******************************************************************
  * @ Name          : void ReceiveError(DataPack_typdef *data_pack)
  * @ Description   : receive error, reset step, save error number 
  * @ Input         : DataPack_typdef *data_pack
  * @ Output        : None
  * @ Return        : None
  * @ Modify the record : ---
******************************************************************/
void ReceiveError(COMM_PARSING_T *pParsing)
{
    pParsing->nErrorNumber = pParsing->nStep;
    pParsing->nStep = 0;
    pParsing->receiveState = RX_Error;
}


uint8_t CalculateCheckSum(COMM_DATA_PACK_T *pPack)
{
    uint8_t checkSum = STX;
    
    checkSum += pPack->nCMD;
    checkSum += (uint8_t)(pPack->nLength & 0xff);
    checkSum += (uint8_t)((pPack->nLength >> 8) & 0xff);
    
    uint32_t i;
    
    for (i = 0; i < pPack->nLength; i ++)
    {
        checkSum += pPack->aData[i];
    }
    
    return checkSum;
}

/************************
 * 转义处理
************************/
static uint16_t DisposeESC(uint32_t nLength, uint8_t *pOut, const uint8_t *pIn)
{
    uint32_t i = 0;
    uint16_t temp;
    uint16_t out_length = 0;
    for (i = 0; i < nLength; i++)
    {
        if (*pIn == STX || *pIn  == ETX || *pIn  == ESC)  //need to  escape data processing
        {
            temp = *pIn | 0x100;
            *pOut = ESC;
            out_length++;
            pOut++;
            *pOut = (uint8_t)((temp - ESC) & 0XFF);
        }
        else
        {
            *pOut = *pIn;
            
        }
        out_length++;
        pOut++;
        pIn++;
    }
    
    return out_length;
}

/******************************************************************
  * @ Name          : void PacketParsing(DataPack_typdef *data_pack, uint8_t data)
  * @ Description   : Packet parsing
  * @ Input         : DataPack_typdef *data_pack
  * @ Output        : None
  * @ Return        : None
  * @ Modify the record : ---
******************************************************************/
void COMM_Parsing(COMM_PARSING_T *pParsing, uint8_t inData)
{
    uint8_t data;
    
    if(inData == STX)     // find STX
    {
        data = inData;
        if (pParsing->nStep != 0)
        {
            ReceiveError(pParsing);
        }
        
    } 
    else if(inData == ESC && pParsing->nLastData != ESC)
    {
        pParsing->nLastData = inData;
        return;
    }
    else if(pParsing->nLastData == ESC)
    {
        data = (uint8_t)((inData + ESC) & 0xFF);
        pParsing->nLastData= 0;
    }
    else
    {
        data = inData;
    }
    

    switch(pParsing->nStep)
    {
        case 0:
        if (data == STX)                    //header check
        {
            pParsing->nStep++;
            pParsing->receiveState = RX_Busy;
        }
        break;
            
            
        case 1:
        {
            pParsing->dataPack.nCMD = data;
            pParsing->nStep++;   
            pParsing->nCount = 0;
        }
        break;    
        
        case 2:
        {
            if (pParsing->nCount == 0)
            {
                pParsing->dataPack.nLength = data;
                pParsing->nCount++;
            }
            else
            {
                pParsing->dataPack.nLength |= (data << 8);
                pParsing->nStep++; 
                pParsing->nCount = 0;
                if (pParsing->dataPack.nLength == 0)
                {
                    pParsing->nStep++; 
                }
            }
        }
        break;
        
        case 3:
        {
            pParsing->dataPack.aData[pParsing->nCount] = data;
            
            pParsing->nCount++;
            if (pParsing->nCount >= pParsing->dataPack.nLength)       //data receive carry out
            {
                pParsing->nStep++;
            }
        }
        break;
        
        case 4:
        {
            if (CalculateCheckSum(&pParsing->dataPack) == data)
            {
                pParsing->nStep++;
            }
            else
            {
                ReceiveError(pParsing);
            }
        }
        break;
        
        
        case 5:
        {
            if (data == ETX)
            {
                pParsing->receiveState = RX_CarryOut;
                pParsing->nStep = 0;
            }
        }
        break;
    }
}

uint8_t escBuf[(COMM_MAX_PACK_DATA_LENGTH + 6) * 2];

/******************************************************************
  * @ Name          : void ReceiveError(DataPack_typdef *pParsing)
  * @ Description   : receive error, reset step, save error number 
  * @ Input         : DataPack_typdef *pParsing
  * @ Output        : None
  * @ Return        : None
  * @ Modify the record : ---
******************************************************************/
void COMM_BuildAndSendPacket(COMM_DATA_PACK_T *pPack)
{
    
    uint32_t length = 0;
    uint8_t checkSum = 0;
    //uint32_t i;
    
    escBuf[length++] = STX;
    length += DisposeESC(1, &escBuf[length], &pPack->nCMD);
    length += DisposeESC(2, &escBuf[length], (uint8_t *)&pPack->nLength);
    length += DisposeESC(pPack->nLength, &escBuf[length], (uint8_t *)&pPack->aData);
    
    
    checkSum = CalculateCheckSum(pPack);
    length += DisposeESC(1, &escBuf[length], &checkSum);
    escBuf[length++] = ETX;
    

    UartSendString(escBuf, length);

}



