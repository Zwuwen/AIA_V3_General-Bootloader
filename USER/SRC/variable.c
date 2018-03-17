#include "variable.h"


u32 U32DEBUG;
u32 Debugi, DebugSize, DebugApp;

/* Used in TIM2 interrupt-------------------------------------------------- */
uint16_t Count_250ms;
uint16_t Count_1s;
uint16_t Count;


/* Used in ACON-AIA-COMM-PROCOTOL-------------------------------------------------- */
unsigned char 	ModuleAdress 		= 	0xff;
char 			ModuleAdressChar 	= 	' ';

unsigned char	SetAdress			= 0x00;	   /*表示设置的地址 即接口板实际的拨码 00 01 10 11*/

FLAG Flag;

FLASH_Status FLASHSTATE;


/**
  * @brief  初始化全局变量
  * @param  None
  * @retval None
  */
void Variable_Init(void)
{
	Flag.Allbits = 0x0000;
	ModuleAdress = 0x01;
	ModuleAdressChar = '1';
}




