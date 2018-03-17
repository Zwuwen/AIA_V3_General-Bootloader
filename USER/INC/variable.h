#ifndef __VARIABLE_H
#define	__VARIABLE_H

#include "stm32f10x.h"

/* Exported macro ------------------------------------------------------------*/

#define 	PASS  							0
#define     FAIL                			1


/** 
  * @brief  bool
  */ 
typedef enum  
{
	FALSE = 0, 
	TRUE
}BOOL;


/** 
  * @brief  Global Flag bit define  
  */ 
typedef union
{
	struct
	{
		uint8_t LedRUNTurnOn			:1; 
		uint8_t LEDLast		:1;
		uint8_t LEDNow		:1; 
		uint8_t startCount	:1;
		uint8_t waittimeout	:1; 
		uint8_t bit5		:1;
		uint8_t bit6		:1; 
		uint8_t Bit7		:1;

		uint8_t CanNewFrame		:1; 
		uint8_t isReceiveChunk		:1;
		uint8_t Bit10		:1; 
		uint8_t Bit11		:1;
		uint8_t Bit12		:1; 
		uint8_t Bit13		:1;
		uint8_t bit14	    :1; 
		uint8_t ForceBootload		:1;
	}Bit;
	uint16_t Allbits;
}FLAG;
extern FLAG Flag;

extern FLASH_Status FLASHSTATE;
extern u32 U32DEBUG;
extern u32 Debugi,DebugSize,DebugApp;
/* Used in TIM2 interrupt-------------------------------------------------- */
extern uint16_t Count_250ms;
extern uint16_t Count_1s;
extern uint16_t Count;




/* Used in ACON-AIA-COMM-PROCOTOL-------------------------------------------------- */
extern unsigned char 	ModuleAdress;
extern char 			ModuleAdressChar;
extern unsigned char	SetAdress;


/* Exported functions ------------------------------------------------------- */
void Variable_Init(void);



#endif
