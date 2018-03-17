#ifndef __FLASH_H
#define	__FLASH_H

#include "variable.h"


extern u32 DecryptKey;


#define FLASH_WRITTEN_SYMBOL 0x5a5a5a5a

BOOL MatchBootCodeAndGetKey(void);
BOOL ReadDontBTFlag(void);
uint8_t CAN_BOOT_ProgramWord(uint32_t Address, uint32_t Data);
void CAN_BOOT_GetAddrData(uint32_t Address,uint8_t *pData);
uint8_t CAN_BOOT_ProgramPage(uint32_t Chunk_Addr, uint32_t Chunk_Size);
#endif
