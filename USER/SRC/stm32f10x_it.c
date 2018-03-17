/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "variable.h"
#include "can.h"
#include "can_bootloader.h"
#include "flash.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/







/**
  * @brief  This function handles TIM2 interrupt request.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{  
	if( TIM_GetITStatus(TIM2,TIM_IT_Update) != RESET ) 	/*10ms*/
   	{
		
		if(Flag.Bit.startCount)
		{
			TimeoutCount--;
			if(TimeoutCount == 0)
			{
				Flag.Bit.waittimeout = 1;
			}
		}

#if BT_LEDFLASH_ENABLE == 1
		Count_1s++;
		if(Count_1s == ((Flag.Bit.isReceiveChunk) ? 80 : 2))	  
		{
			LED_RUN( 1 )
		}
		else if(Count_1s == 1)
		{		
			LED_RUN( 0 )	
		}
		if(Count_1s > 89)
		{
			Count_1s = 0;
		}
#endif
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update); 
    }
}




/**
  * @brief  This function handles TIM3 interrupt request.
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)
{  
}



/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	u32 DecryptedByte;
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);


	if (RxMessage.IDE != CAN_ID_EXT)
		 return;

    if ((RxMessage.ExtId&0x00ff) == ModuleAdress)
    {
		CmdType = (CMDTYPE)((RxMessage.ExtId>>8)&0x0f);
		switch(CmdType)
		{
			case CMD1_CHUNK_START:
				if(Flag.Bit.isReceiveChunk)	break;
				Chunk_Seq = U32_CANFRAME67;
				if(Chunk_Seq != CurrentSeq) break;
				Chunk_Addr = U32_CANFRAME0123;
				Chunk_Size = U32_CANFRAME45;
				Flag.Bit.CanNewFrame = 1;
			break;

			case CMD2_CHUNK_WRITE:
				if(!Flag.Bit.isReceiveChunk) break;
				if(ChunkReadIndex > (Chunk_Size-1))
				{
					break;
				}
				DecryptedByte = U32_CANFRAME0123 ^ DecryptKey;
				APPCode[ChunkReadIndex++] = DecryptedByte;
				ChunkSum += DecryptedByte;
				if(ChunkReadIndex > (Chunk_Size-1))
				{
					break;
				}
				DecryptedByte = U32_CANFRAME4567 ^ DecryptKey;
				APPCode[ChunkReadIndex++] = DecryptedByte;
				ChunkSum += DecryptedByte;
				 
			break;
			case CMD3_CHUNK_END:
				if(Flag.Bit.isReceiveChunk)
				{
					ChunkEnd1 = U32_CANFRAME0123;
					ChunkEnd2 = U32_CANFRAME4567;
					Flag.Bit.CanNewFrame = 1;
				}
			break;
			
			case CMD4_GET_VERSION:
				Flag.Bit.CanNewFrame = 1;
			break;

			case CMD5_COMPLETE:
				Run_Addr = U32_CANFRAME0123;
				Run_Check = U32_CANFRAME4567;
				if(Run_Addr>= FLASH_ADDR_APPLICATION)
				{
					Flag.Bit.CanNewFrame = 1;
				}
			break;
					
		default:
			break;
		} 
    }


 	if(Flag.Bit.CanNewFrame) return; 
}





/**
  * @brief  This function handles USART3 Read interrupt request.  Use ACON-AIA-COMM-PROTOCOL
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{

}

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
