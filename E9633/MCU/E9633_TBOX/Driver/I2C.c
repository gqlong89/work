#include "I2C.h"




void I2C_GPIO_Configuration(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;
	
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	
	  //����SCL����
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;   
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	  //����SDA����	  
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;   
    GPIO_Init(GPIOB, &GPIO_InitStructure);

}

void I2C_Configuration(void)
{
		I2C_InitTypeDef I2C_InitStructure;
	
	 
	  I2C_GPIO_Configuration();
	
	  I2C_DeInit(I2C1);
	  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	  I2C_InitStructure.I2C_DutyCycle  = I2C_DutyCycle_2;
	  I2C_InitStructure.I2C_OwnAddress1 = 0x0A;
	  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	  I2C_InitStructure.I2C_ClockSpeed = 100000;//400000  200000

	  I2C_Init(I2C1,&I2C_InitStructure);
	  I2C_Cmd(I2C1,ENABLE);	
	  I2C_AcknowledgeConfig( I2C1, ENABLE ); 
}

