/**
  ******************************************************************************
  * @file    iic_driver.c 
  * @author  Bowen.He
  * @version V1.1
  * @date    20-November-2015
  * @brief   IIC Driver:  Each while statement has timeout.
  ******************************************************************************
  * @attention
  *
  * A typical IIC Driver contains 2 files. There are :
  *	iic_driver.c: six functions. I2C_Configuration should be called before other
  *			  	  functions were used.
  * iic_driver.h: In this file, user can modify some configuration.
  * 
  * <h2><center>&copy; COPYRIGHT 2015 ACONBIO</center></h2>
  ******************************************************************************/
  
#include "iic_driver.h"

void I2C_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;	

    RCC_APB2PeriphClockCmd(I2C_RCC_APBPeriph_GPIO, ENABLE);
    RCC_APB1PeriphClockCmd(I2C_RCC_APBPeriph_I2C, ENABLE);

    GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(I2C_GPIO, &GPIO_InitStructure);

    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;				
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;		
    I2C_InitStructure.I2C_OwnAddress1 = I2C2_SLAVE_ADDRESS7;	
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;				
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;		
	
    I2C_Init(I2C_CHANNEL, &I2C_InitStructure);
    I2C_Cmd(I2C_CHANNEL, ENABLE);	
}


#define WHILE_WITH_TIMEOUT(X) while((X) && (--i > 0)); if(i == 0) return FALSE; else i = I2CWAITCOUNT
BOOL I2C_EE_ByteWrite(u8* pBuffer, u8 WriteAddr)
{
	int i;
	
	i = I2CWAITCOUNT;
	
	if(I2C_EE_WaitEepromStandbyState() == FALSE) return FALSE;
	
    I2C_GenerateSTART(I2C_CHANNEL, ENABLE);
	WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_MODE_SELECT));
	
    I2C_Send7bitAddress(I2C_CHANNEL, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));


    I2C_SendData(I2C_CHANNEL, WriteAddr);
    WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C_CHANNEL, *pBuffer);
    WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTOP(I2C_CHANNEL, ENABLE);
	
	return TRUE;
}



BOOL I2C_EE_WaitEepromStandbyState(void)
{
	int i;
    vu16 SR1_Tmp = 0;
	i = I2CWAITCOUNT;
	
    do
    {
        I2C_GenerateSTART(I2C_CHANNEL, ENABLE);
        SR1_Tmp = I2C_ReadRegister(I2C_CHANNEL, I2C_Register_SR1);
        I2C_Send7bitAddress(I2C_CHANNEL, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    } while ((!(I2C_ReadRegister(I2C_CHANNEL, I2C_Register_SR1) & 0x0002)) && (--i > 0));
	if(i == 0) return FALSE;
	
    I2C_ClearFlag(I2C_CHANNEL, I2C_FLAG_AF);
	return TRUE;

}


BOOL I2C_EE_BufferRead(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead)
{
	int i;
	int num;
	i = I2CWAITCOUNT;
	num =  NumByteToRead;
	
    if(I2C_EE_WaitEepromStandbyState() == FALSE) return FALSE;
	
    I2C_GenerateSTART(I2C_CHANNEL, ENABLE);
    WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_MODE_SELECT));
	
    if (num==1)
    {
        I2C_AcknowledgeConfig(I2C_CHANNEL, DISABLE);
    }
	
    I2C_Send7bitAddress(I2C_CHANNEL, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C_CHANNEL, ReadAddr);
    WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTART(I2C_CHANNEL, ENABLE);
    WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C_CHANNEL, EEPROM_ADDRESS, I2C_Direction_Receiver);
    WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    while(num)
    {
        if (I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_BYTE_RECEIVED))
        {
            if (num == 2)
            {
                I2C_AcknowledgeConfig(I2C_CHANNEL, DISABLE);
            }

            if (num == 1)
            {
                I2C_GenerateSTOP(I2C_CHANNEL, ENABLE);
            }

            *pBuffer = I2C_ReceiveData(I2C_CHANNEL);
            pBuffer++;
            num--;
        }
    }
    I2C_AcknowledgeConfig(I2C_CHANNEL, ENABLE);
	return TRUE;
}




BOOL I2C_EE_PageWrite(u8* pBuffer, u8 WriteAddr, u8 NumByteToWrite)
{
	int i;
	i = I2CWAITCOUNT;
	
	if(I2C_EE_WaitEepromStandbyState() == FALSE) return FALSE;
	
    I2C_GenerateSTART(I2C_CHANNEL, ENABLE);
    WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_MODE_SELECT));
	
    I2C_Send7bitAddress(I2C_CHANNEL, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C_CHANNEL, WriteAddr);
    WHILE_WITH_TIMEOUT(! I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    while (NumByteToWrite--)
    {
        I2C_SendData(I2C_CHANNEL, *pBuffer);
        pBuffer++;
        WHILE_WITH_TIMEOUT(!I2C_CheckEvent(I2C_CHANNEL, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }
    I2C_GenerateSTOP(I2C_CHANNEL, ENABLE);
	return TRUE;
}






BOOL I2C_EE_BufferWrite(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite)
{
	int NumOfPage, NumOfSingle, Addr, count;
	
    NumOfPage = 0;
	NumOfSingle = 0;
	Addr = 0;
	count = 0;
	
    Addr = WriteAddr % I2C_PageSize;
    count = I2C_PageSize - Addr;
    NumOfPage = NumByteToWrite / I2C_PageSize;

    NumOfSingle = NumByteToWrite % I2C_PageSize;

    //if(I2C_EE_WaitEepromStandbyState() == FALSE) return FALSE;
	
    if (Addr == 0)
    {
        if (NumOfPage == 0)
        {
            if(I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle) == FALSE) return FALSE;
        }
        else
        {
            while (NumOfPage--)
            {
                if(I2C_EE_PageWrite(pBuffer, WriteAddr, I2C_PageSize) == FALSE) return FALSE;
                WriteAddr += I2C_PageSize;
                pBuffer += I2C_PageSize;
            }
            if (NumOfSingle!=0)
            {
                if(I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle) == FALSE) return FALSE;
            }
        }
    }
    else   
    {
        if (NumOfPage== 0)
        {
            if(I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle) == FALSE) return FALSE;
        }
        else
        {
            NumByteToWrite -= count;
            NumOfPage = NumByteToWrite / I2C_PageSize;         
            NumOfSingle = NumByteToWrite % I2C_PageSize;

            if (count != 0)
            {
                if(I2C_EE_PageWrite(pBuffer, WriteAddr, count) == FALSE) return FALSE;
                WriteAddr += count;
                pBuffer += count;
            }
            while (NumOfPage--)
            {
                if(I2C_EE_PageWrite(pBuffer, WriteAddr, I2C_PageSize) == FALSE) return FALSE;
                WriteAddr += I2C_PageSize;
                pBuffer += I2C_PageSize;
            }
            if (NumOfSingle != 0)
            {
				if(I2C_EE_PageWrite(pBuffer, WriteAddr, NumOfSingle) == FALSE) return FALSE;
            }
        }
    }
	return TRUE;
}























