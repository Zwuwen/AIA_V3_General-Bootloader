#include "can_bootloader.h"
#include "flash.h"
#include "string.h"


CMDTYPE CmdType;
u32 Erase_Start, Erase_Size;
u32 Write_Addr, Write_Data;
u32 Chunk_Addr;	
u16 Chunk_Size, Chunk_Seq;
u32 ChunkEnd1, ChunkEnd2;
u32 Run_Addr, Run_Check;
u32 Read_Addr;
u32 Go_Addr;

u32 TimeoutCount;

u32 APPCode[512]; 		/*pagesize/4*/
u32 ChunkSumXor;  		/*和校验的异或*/
u32 ChunkReadIndex; 	/*读取索引*/
u32 ChunkSum; 			/*求和校验*/
u16 CurrentSeq; 		/*当前序列*/


/**
  * @brief 
  * @param 
  * @retval 
  */
void CAN_BOOT_JumpToApplication(uint32_t Addr)
{
	pFunction Jump_To_Application;
	__IO uint32_t JumpAddress; 
	/* Test if user code is programmed starting from address "ApplicationAddress" */
	if (((*(__IO uint32_t*)Addr) & 0x2FFE0000 ) == 0x20000000)
	{ 
	  /* Jump to user application */
	  JumpAddress = *(__IO uint32_t*) (Addr + 4);
	  Jump_To_Application = (pFunction) JumpAddress;
		__disable_irq();
		FLASH_Lock();
	  /* Initialize user application's Stack Pointer */
	  __set_MSP(*(__IO uint32_t*) Addr);
	  Jump_To_Application();
	}
}



#if(BT_LEDFLASH_ENABLE == 1)
void LED_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( BT_RCC_LEDGPIO, ENABLE);  

	GPIO_InitStructure.GPIO_Pin = BT_LEDPIN;	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(BT_LEDGPIO, &GPIO_InitStructure);	
}
#endif


/**
  * @brief 
  * @param  
  * @retval
  */
void SendCanResp(CMDTYPE Cmd, u8 Status)
{
	u8 TransmitMailbox = 0;	
	CanTxMsg TxMessage;
	TxMessage.StdId =  0;
	TxMessage.ExtId = (ModuleAdress | ((u16)Cmd<<8));	
	TxMessage.RTR 	= CAN_RTR_DATA;	
	TxMessage.IDE 	= CAN_ID_EXT;
	TxMessage.DLC   = 1;
	TxMessage.Data[0] = Status;
	TransmitMailbox = CAN_Transmit(CAN1,&TxMessage);
	while(CAN_TransmitStatus(CAN1,TransmitMailbox) != CANTXOK);
}




/**
  * @brief  
  * @param  None
  * @retval None
  */
void CANSendString(char *pStr,uint32_t Identifier)
{	
	u8 TransmitMailbox = 0;
	CanTxMsg TxMessage;
	u8 StrLength = strlen(pStr);
	u8 RemainNumber = StrLength;
	u8 i;

	while(RemainNumber != 0)	                                                                             
	{
	   TxMessage.StdId = 0; 	
	   TxMessage.ExtId = Identifier;		
	   TxMessage.RTR 	= CAN_RTR_DATA;	
	   TxMessage.IDE 	= CAN_ID_EXT;
	   if(RemainNumber > 8)
	   {
	      TxMessage.DLC	= 8;
		  
		  for(i=0;i<8;i++)
		  {
		     TxMessage.Data[i] = pStr[StrLength - RemainNumber + i];			
		  }
          TransmitMailbox = CAN_Transmit(CAN1,&TxMessage);	 		
		  while(CAN_TransmitStatus(CAN1,TransmitMailbox) != CANTXOK);

		  RemainNumber -= 8;  					
	   }
	   else
	   {
	      TxMessage.DLC 	= RemainNumber;	

		  for(i=0;i<RemainNumber;i++)
		  {
		     TxMessage.Data[i] = pStr[StrLength - RemainNumber + i];	
		  }
          TransmitMailbox = CAN_Transmit(CAN1,&TxMessage);	 		
		  while(CAN_TransmitStatus(CAN1,TransmitMailbox) != CANTXOK);

		  RemainNumber = 0; 
	   }	
	}	
}




/**
  * @brief  
  * @param 
  * @retval 
  */
void ProcessCanCMD(void)
{
	u8 CmdExcuStatus = 0;
	char AnswerStr[8];

	if(!Flag.Bit.CanNewFrame) return;   /*Processing now*/

	switch(CmdType)
	{
		case CMD1_CHUNK_START:
			if((Chunk_Addr>= FLASH_ADDR_APPLICATION) && (Chunk_Size <= 512))
			{
				CmdExcuStatus = 1;	
				Flag.Bit.isReceiveChunk = 1;
				ChunkReadIndex = 0;
				ChunkSum = 0;		
			}
			SendCanResp(CMD1_CHUNK_START, CmdExcuStatus);			
		break;

		case CMD3_CHUNK_END:
			CmdExcuStatus = 2;
			if(ChunkSum == (ChunkEnd1 ^ ChunkEnd2))	/*Check ok*/
			{										/*Make sure Chunk_Addr not be modified before used*/
				Flag.Bit.isReceiveChunk = 0;  
				CmdExcuStatus = 3;
				if(CAN_BOOT_ProgramPage(Chunk_Addr,Chunk_Size))
				{
					CmdExcuStatus = 1;
					CurrentSeq++;
					ChunkSumXor ^= ChunkSum;
				}								
			}
			SendCanResp(CMD3_CHUNK_END, CmdExcuStatus);
		break;

		case CMD4_GET_VERSION:
			Flag.Bit.startCount = 0;  /*No need to count time out. It's start bt now*/
			CmdExcuStatus = 1;
			AnswerStr[0] = CmdExcuStatus;
			AnswerStr[1] = BT_FLASH_PAGESIZE/1024 +'0'; /*Page Size*/
			AnswerStr[2] = 'K';
			AnswerStr[3] = 'U';  /*UserApp*/
			AnswerStr[4] = FIRST_VER;  /*UserApp*/
			AnswerStr[5] = MIDDLE_VER;  /*UserApp*/
			AnswerStr[6] = TEMP_VER;  /*UserApp*/
			AnswerStr[7] = '\0';							
			CANSendString(AnswerStr, (ModuleAdress | ((u16)CMD4_GET_VERSION<<8)));
		break;

		case CMD5_COMPLETE:
			if(ChunkSumXor == Run_Check)
			{
				if ( (U32_VALUE_APPHEAD & 0x2FFE0000)  == 0x20000000)
				{				
					CmdExcuStatus = CAN_BOOT_ProgramWord(FLASH_ADDR_BOOTCODE, 0)? 1 : 0;
					FLASH_Lock();
					SendCanResp(CMD5_COMPLETE, CmdExcuStatus);
				}	
				/* reset*/
				__disable_fault_irq();  
				NVIC_SystemReset();	
			}
			SendCanResp(CMD5_COMPLETE, 2);			
		break;
		
		default:
		break;
	}
	Flag.Bit.CanNewFrame = 0; 	
}

