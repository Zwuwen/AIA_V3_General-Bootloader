#include "can.h"
#include "string.h"
#include "can_bootloader.h"



CanRxMsg RxMessage;


/**
  * @brief  配置CAN通讯相关的寄存器
  * @param  None
  * @retval None
  */
static void CAN_Configuration(void)
{
	CAN_InitTypeDef        CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
    GPIO_InitTypeDef  	   GPIO_InitStructure;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CAN_TX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIO_CAN, &GPIO_InitStructure);	  
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CAN_RX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIO_CAN, &GPIO_InitStructure);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);


   /*CAN 寄存器初始化*/
    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);

    /*CAN 初始化*/
    CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = DISABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;	
    CAN_InitStructure.CAN_RFLM = DISABLE;	
    CAN_InitStructure.CAN_TXFP = DISABLE;
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; 
    CAN_InitStructure.CAN_SJW = CAN_SJW_2tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS1_16tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
    CAN_InitStructure.CAN_Prescaler = 18;
    CAN_Init(CAN1,&CAN_InitStructure);
	
	/*只接受6个type类型，防止总线上数据干扰，特别是泵的指令的干扰*/
    CAN_FilterInitStructure.CAN_FilterNumber 		= 	0;
    CAN_FilterInitStructure.CAN_FilterMode 			= 	CAN_FilterMode_IdList;
    CAN_FilterInitStructure.CAN_FilterScale 		= 	CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh   		= 	(((u32)((CMD1_CHUNK_START<<8) | ModuleAdress)<<3)&0xffff0000)>>16;	
	CAN_FilterInitStructure.CAN_FilterIdLow   		= 	(((u32)((CMD1_CHUNK_START<<8) | ModuleAdress)<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xffff;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh  	= 	(((u32)((CMD2_CHUNK_WRITE<<8) | ModuleAdress)<<3)&0xffff0000)>>16;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow   	= 	(((u32)((CMD2_CHUNK_WRITE<<8) | ModuleAdress)<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xffff;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 	0;
    CAN_FilterInitStructure.CAN_FilterActivation = 		ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);
	
    CAN_FilterInitStructure.CAN_FilterNumber 		= 	1;
    CAN_FilterInitStructure.CAN_FilterMode 			= 	CAN_FilterMode_IdList;
    CAN_FilterInitStructure.CAN_FilterScale 		= 	CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh   		= 	(((u32)((CMD3_CHUNK_END<<8) | ModuleAdress)<<3)&0xffff0000)>>16;
	CAN_FilterInitStructure.CAN_FilterIdLow   		= 	(((u32)((CMD3_CHUNK_END<<8) | ModuleAdress)<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xffff;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh  	= 	(((u32)((CMD4_GET_VERSION<<8) | ModuleAdress)<<3)&0xffff0000)>>16;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow   	= 	(((u32)((CMD4_GET_VERSION<<8) | ModuleAdress)<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xffff;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 	0;
    CAN_FilterInitStructure.CAN_FilterActivation = 		ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);
	
    CAN_FilterInitStructure.CAN_FilterNumber 		= 	2;
    CAN_FilterInitStructure.CAN_FilterMode 			= 	CAN_FilterMode_IdList;
    CAN_FilterInitStructure.CAN_FilterScale 		= 	CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh   		= 	(((u32)((CMD5_COMPLETE<<8) | ModuleAdress)<<3)&0xffff0000)>>16;
	CAN_FilterInitStructure.CAN_FilterIdLow   		= 	(((u32)((CMD5_COMPLETE<<8) | ModuleAdress)<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xffff;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh  	= 	(((u32)((CMD0_RESERVE<<8) | ModuleAdress)<<3)&0xffff0000)>>16;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow   	= 	(((u32)((CMD0_RESERVE<<8) | ModuleAdress)<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xffff;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 	0;
    CAN_FilterInitStructure.CAN_FilterActivation = 		ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);
}





/**
  * @brief  初始化CAN1模块
  * @param  None
  * @retval None
  */
void Can_Init(void)
{
    CAN_Configuration();
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);	
}
