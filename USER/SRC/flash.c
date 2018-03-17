#include "flash.h"
#include "can_bootloader.h"
#include "string.h"

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

u32 DecryptKey;

u32 EraseCounter = 0x00, Address = 0x00;
volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
volatile TestStatus MemoryProgramStatus = PASSED;

u8 NbrOfPage = 0x00;

BOOL MatchBootCodeAndGetKey(void)
{
	u32 tmp32;
	tmp32 = (*((__IO u32*)FLASH_ADDR_BOOTCODE));
	DecryptKey = ((ModuleAdress<<24)| (ModuleAdress<<16)| (ModuleAdress<<8)| ModuleAdress)
				 ^ ((u32)(('A'<<24)|('C'<<16)|('O'<<8)|'N'));

	if(DecryptKey == tmp32)
		return TRUE;
	else return FALSE;
}



BOOL ReadDontBTFlag(void)
{
	u32 tmp32;
	tmp32 = (*((__IO u32*)(FLASH_ADDR_BOOTCODE-4)));
	if(BT_DONTBT_FLAG == tmp32)
		return TRUE;
	else return FALSE;
}




uint8_t CAN_BOOT_ProgramWord(uint32_t Address, uint32_t Data)
{
	volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;


	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	if(FLASH_COMPLETE != FLASH_ErasePage(Address)) return 0;	/*擦除一页*/

	FLASHStatus = FLASH_ProgramWord(Address,Data);
	if(FLASHStatus == FLASH_COMPLETE)
		return	1;	
	else
		return	0;	
}





/**
  * @brief  从指定的Flash区读取一个字的数据
  * @param  Address 开始读取数据的地址。
	* @param  pData 数据储存地址。
  * @retval 程序跳转状态。
  */
void CAN_BOOT_GetAddrData(uint32_t Address,uint8_t *pData)
{
	*pData++ = *(__IO uint8_t*)Address++;
	*pData++ = *(__IO uint8_t*)Address++;
	*pData++ = *(__IO uint8_t*)Address++;
	*pData++ = *(__IO uint8_t*)Address++;
}


/**
  * @brief  将程序数据写入指定Flash地址
  * @param  Address 起始地址
  * @param  Data 准备写入的数据
  * @retval 数据写入状态，0-写入数据成功，1-写入数据失败
  */
uint8_t CAN_BOOT_ProgramPage(uint32_t Addr, uint32_t Size)
{
	u32 i = 0;
	volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;

	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	if(FLASH_COMPLETE != FLASH_ErasePage(Addr)) return 0;	/*擦除一页*/
	
	/* Program Flash Bank1 */
	while((i < Size) && (FLASHStatus == FLASH_COMPLETE))
	{
		FLASHStatus = FLASH_ProgramWord(Addr + i*4 , APPCode[i]);
		FLASHSTATE = FLASHStatus;
		i++;
	}

	if(i == Size)
		return 1;
	else
	{ 
		if(FLASHSTATE) U32DEBUG++;
		return 0;
	}
}




