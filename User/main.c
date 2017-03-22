#include "sys.h"
#include "usart.h"
#include "Communcation.h"
#include "ActionProcess.h"

COMM_PARSING_T comParsing;


int main(void)
{
    uart_init(8,115200);
    
    while(1)
    {
        if(comParsing.receiveState == RX_CarryOut)
        {
            Action_PacketProcess(&comParsing.dataPack);
            comParsing.receiveState = RX_Free;
        }
    }
}

void ReceiveParsing(uint8_t nData)
{
    COMM_Parsing(&comParsing, nData);
}









