/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
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
#include "stm32f10x.h"
#include "variable.h"
#include "can.h"
#include "flash.h"
#include "can_bootloader.h"
#include "iic_driver.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void TIME2_Config(void);
void NVIC_Config(void);



void ChangeAddrType(void)
{
	if((0 < ModuleAdress) && (9 >= ModuleAdress))
	{
		ModuleAdressChar = ModuleAdress + '0';
	}
	else if((10 <= ModuleAdress) && (35 >= ModuleAdress))
	{
		ModuleAdressChar = 'A' + (ModuleAdress - 10);
	}
	else if((36 <= ModuleAdress) && (61 >= ModuleAdress))
	{
		ModuleAdressChar = ModuleAdress + 61;
	}
}





void GetModuleAddress(void)
{
	u32 key;
	u8 data;
	key = 0;
	data = 0;
	
#if ((BT_PARAM_SOURCE == I2C_1) || (BT_PARAM_SOURCE == I2C_2))	
	I2C_EE_BufferRead((u8*)&key, 0x00, 4);
	
	if(key == I2C_KEY)
	{
		I2C_EE_BufferRead(&data, 0x04, 1);
	}
#else
	if((*((__IO u32*)FLASH_ADDR_SYMBOL)) == FLASH_WRITTEN_SYMBOL)
	{
		flashdata = (*((__IO u32*)FLASH_ADDR_PARAM));
		data = flashdata;
	}
#endif

	if((data>0) &&(data < 62))
	{
		ModuleAdress = data;
		ChangeAddrType();
	}
	else
	{
		ModuleAdress = 1;
		ModuleAdressChar = '1';
		Flag.Bit.ForceBootload = 0;
	}
}




u8 WriteDontBootLoadFlagIntoFlash(void)
{
	u32 boot;

	volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
	FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	boot = U32_VALUE_BOOTCODE;
    FLASHStatus = FLASH_ErasePage(FLASH_ADDR_BOOTCODE);
	if(FLASHStatus != FLASH_COMPLETE) {FLASH_Lock(); return 0;}

	FLASHStatus = FLASH_ProgramWord(FLASH_ADDR_BOOTCODE-4, BT_DONTBT_FLAG);
	FLASHStatus = FLASH_ProgramWord(FLASH_ADDR_BOOTCODE, boot);
	
	if(FLASHStatus != FLASH_COMPLETE) {FLASH_Lock(); return 0;}
    FLASH_Lock();

	return 1;
}




/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
	SystemInit();
	
#if ((BT_PARAM_SOURCE == I2C_1) || (BT_PARAM_SOURCE == I2C_2))		
	I2C_EE_Init();
#endif
	
	Variable_Init();
	GetModuleAddress();
	

	if(ReadDontBTFlag()) CAN_BOOT_JumpToApplication(FLASH_ADDR_APPLICATION);
	
	if((Flag.Bit.ForceBootload == 1) ||
		MatchBootCodeAndGetKey() || 						
	  ((U32_VALUE_APPHEAD & 0x2FFE0000)  != 0x20000000)) 
	{
		/*Start Bootloader*/
		FLASH_Unlock();			
		Can_Init();				
		TIME2_Config();			
		NVIC_Config();
#if BT_LEDFLASH_ENABLE == 1	
		LED_GPIO_Config();	
#endif		
		ChunkSumXor = 0;	
		CurrentSeq = 0;		
		ChunkSum = 0;		
		TimeoutCount = BT_WAIT_TIMEOUT_S;
		Flag.Allbits = 0x0000;
		Flag.Bit.startCount = 1;
		
		while(1)
		{
			if(Flag.Bit.CanNewFrame)
			{
				ProcessCanCMD();
				Flag.Bit.CanNewFrame = 0;	
			}
			if(Flag.Bit.waittimeout)
			{
				Flag.Bit.startCount = 0;
				Flag.Bit.waittimeout = 0;
				WriteDontBootLoadFlagIntoFlash();
				/* reset*/
				__disable_fault_irq();  
				NVIC_SystemReset();						
			}
		}	  	  	
	}
	else
	{
		CAN_BOOT_JumpToApplication(FLASH_ADDR_APPLICATION);
	}

  /* Infinite loop */
	while (1)
	{
	}

}





/**
  * @brief 
  * @param  None
  * @retval None
  */
void TIME2_Config(void)   					
{  
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE); 
	TIM_DeInit(TIM2);            					
	TIM_TimeBaseStructure.TIM_Period=10000;  				/*10ms*/
	TIM_TimeBaseStructure.TIM_Prescaler=(72-1);          	
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);   
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);          
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);  		
	TIM_Cmd(TIM2,ENABLE);            				
}





/**
  * @brief  ÖÐ¶ÏÅäÖÃ
  * @param  None
  * @retval None
  */
void NVIC_Config(void)   
{  NVIC_InitTypeDef  NVIC_InitStructure;  

   #ifdef  VECT_TAB_RAM  
	/* Set the Vector Table base location at 0x20000000 */ 
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
    #else  /* VECT_TAB_FLASH  */
	/* Set the Vector Table base location at 0x08000000 */ 
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
    #endif

	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
         

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;   
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;   
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 

	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
