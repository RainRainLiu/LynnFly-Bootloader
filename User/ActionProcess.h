#ifndef __ACTION_PROCESS_H__
#define __ACTION_PROCESS_H__
#include <stm32f10x.h>
#include "Communcation.h"

void Action_PacketProcess(COMM_DATA_PACK_T *pPacket);

void Action_AutoRun(void);

#endif

