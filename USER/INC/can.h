#ifndef __CAN_H
#define	__CAN_H

#include "variable.h"
	  
//CANπ‹Ω≈≈‰÷√
#define GPIO_CAN                   GPIOA
#define RCC_APB2Periph_GPIO_CAN    RCC_APB2Periph_GPIOA
#define GPIO_Pin_CAN_RX            GPIO_Pin_11
#define GPIO_Pin_CAN_TX            GPIO_Pin_12


/* Exported variables ------------------------------------------------------- */
//extern CanTxMsg TxMessage;
extern CanRxMsg RxMessage;


/* Exported functions ------------------------------------------------------- */
void Can_Init(void);
void CANSendString(char *pStr,uint32_t Identifier);
void CANSend8Byte(char *pStr,uint32_t Identifier);

#endif
