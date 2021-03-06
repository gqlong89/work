/*
********************************************************************************
*  Copyright (C), 2009-2013, 
*
*  项目名称：xxxx
*  
*  文件名称: xxxx.c
*
*  文件描述：xxxx
*             
*             
*  创 建 者:
*
*  创建日期：2017-12-11 
*
*  版 本 号：V1.0
*
*  修改记录： 
*             
*      1. 日    期： 
*         修 改 人： 
*         所作修改： 
*      2. ...
********************************************************************************
*/
#include "Adc.h"
#include "Uart.h"
#include "Exti.h"
#include "menu.h"
#include "sleep.h"
#include "Timer.h"
#include "shell.h"
#include "Delay.h"
#include "Flash.h"
#include "ymodem.h"
#include "flash_if.h"
#include "Watchdog.h"
#include "bsp_iwdg.h"
#include "UartProcess.h"
#include "lis331dlh_driver.h"
#include "System_stm32f10x.h"
/*
********************************************************************************                                                                  
*  外部变量声明                                                                                                       
********************************************************************************
*/

bool is_jiasu = FALSE;
bool is_jiansu = FALSE;

extern u8 Rollover[7];
extern bool is_accon_ack;
extern bool is_accoff_ack;
extern bool is_rollover_ack;
extern bool is_overturned_ack;
extern float Build_In_Battery;

AxesRaw_t data;
u32 Timer3Count = 0;

//唤醒速锐得
u8 wakedevice[2] = {0x01,0x02};
//急加速急减速
u8 accelerate[6] = {0x1A, 0x00, 0x01};
//点火
u8 turnoncmd[6] = {0x06, 0x00, 0x01,0x00};
//熄火
u8 turnoffcmd[6] = {0x06, 0x00, 0x01, 0x01};

extern status_t lis331dlh_Init;

void TIMER2_Configuration(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE); /* 时钟使能*/
    TIM_DeInit(TIM2); /*重新将Timer设置为缺省值*/
    TIM_TimeBaseStructure.TIM_Period=20000-1; /*自动重装载寄存器周期的值(计数  值) 1s*/
    TIM_TimeBaseStructure.TIM_Prescaler= (uint16_t) (SystemCoreClock / 20000) - 1;  /* 时钟预分频数 72M/20000 -1 */  
 
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;  /* 采样分频 */  
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; /* 向上计数模式 */  
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);  /* 清除溢出中断标志 */    
	
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;            
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0A;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;          
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  
}
/*
********************************************************************************
*  函数名称: TIMER3_Configuration
*
*  功能描述: 定时器初始化
*
*  输入参数: 无
*
*  输出参数: 无
*
*  返 回 值: 无
*
********************************************************************************
*/
void TIMER3_Configuration(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE); /* 时钟使能*/
    TIM_DeInit(TIM3); /*重新将Timer设置为缺省值*/
	
    TIM_TimeBaseStructure.TIM_Period=20000-1; /*自动重装载寄存器周期的值(计数  值) 1s*/
    TIM_TimeBaseStructure.TIM_Prescaler= (uint16_t) (SystemCoreClock / 20000) - 1;  /* 时钟预分频数 72M/20000 -1 */  
 
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;  /* 采样分频 */  
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; /* 向上计数模式 */  
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);  
	
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);  /* 清除溢出中断标志 */    
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);  
  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;            
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4; //优先级最高
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;          
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  
}
/*
********************************************************************************
*  函数名称: TIMER4_Configuration
*
*  功能描述: 定时器初始化
*
*  输入参数: 无
*
*  输出参数: 无
*
*  返 回 值: 无
*
********************************************************************************
*/
void TIMER4_Configuration(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE); /* 时钟使能*/
    TIM_DeInit(TIM4); /*重新将Timer设置为缺省值*/
	
    TIM_TimeBaseStructure.TIM_Period=20000-1; /*自动重装载寄存器周期的值(计数  值) 1s*/
    TIM_TimeBaseStructure.TIM_Prescaler= (uint16_t) (SystemCoreClock / 20000) - 1;  /* 时钟预分频数 72M/20000 -1 */  
 
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;  /* 采样分频 */  
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; /* 向上计数模式 */  
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);  
	
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);  /* 清除溢出中断标志 */    
    TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);  
  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;            
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4; //优先级最高
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;          
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  
}
/*
********************************************************************************
*  函数名称: TIMER5_Configuration
*
*  功能描述: 定时器初始化
*
*  输入参数: 无
*
*  输出参数: 无
*
*  返 回 值: 无
*
********************************************************************************
*/
void TIMER5_Configuration(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5 , ENABLE); /* 时钟使能*/
    TIM_DeInit(TIM5); /*重新将Timer设置为缺省值*/

    TIM_TimeBaseStructure.TIM_Period=20000-1; /*自动重装载寄存器周期的值(计数  值) 1s*/
    TIM_TimeBaseStructure.TIM_Prescaler= (uint16_t) (SystemCoreClock / 20000) - 1;  /* 时钟预分频数 72M/20000 -1 */  
 
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;  /* 采样分频 */  
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; /* 向上计数模式 */  
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);  
	
    TIM_ClearFlag(TIM5, TIM_FLAG_Update);  /* 清除溢出中断标志 */    
    TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);  
  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;            
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //优先级最低 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;          
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  
}
/*
********************************************************************************
*  函数名称: TIMER6_Configuration
*
*  功能描述: 定时器初始化
*
*  输入参数: 无
*
*  输出参数: 无
*
*  返 回 值: 无
*
********************************************************************************
*/
void TIMER6_Configuration(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6 , ENABLE); /* 时钟使能*/
    TIM_DeInit(TIM6); /*重新将Timer设置为缺省值*/
	
    TIM_TimeBaseStructure.TIM_Period=20000-1; /*自动重装载寄存器周期的值(计数  值) 1s*/
    TIM_TimeBaseStructure.TIM_Prescaler= (uint16_t) (SystemCoreClock / 20000) - 1;  /* 时钟预分频数 72M/20000 -1 */  
 
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;  /* 采样分频 */  
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; /* 向上计数模式 */  
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);  
	
    TIM_ClearFlag(TIM6, TIM_FLAG_Update);  /* 清除溢出中断标志 */    
    TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);  
  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  
    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;            
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;          
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  
}
/*
定时器3中断处理
*/
void TIM3_IRQHandler(void)   
{
		u8 rdata;
	  static u8 count = 0;
	 
    //每3秒检测ACC状态
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  
		{
			   count++;
			   if(count == 3)
				 {
					   Feeddog();
						 count = 0;
						 rdata = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1);
						 if(LOW_LEVEL == rdata){
                  is_accoff_ack = FALSE;
							    Timer3Count= 0;
									//若没有ACC ON应答，则一直发ACC ON
									if(is_accon_ack == FALSE)
									{
										  senddatapacket(&COM2,turnoncmd,FIXED_LENGTH+1+CHECKSUM);
							    }
									if(sleepmode == SYSTEM_LIGHT_SLEEP)
									{
											quit_light_sleep();
									}
							}
							else if(LOW_LEVEL != rdata){
											is_accon_ack = FALSE;
								      //唤醒速锐得
											GPIO_SetBits(GPIOA, GPIO_Pin_8);
											SendRS485(&COM3,wakedevice,2);
											DelayMS(50);
											GPIO_ResetBits(GPIOA, GPIO_Pin_8);
											if(Build_In_Battery>3.7 &&sleepmode == SYSTEM_RUN)
											{
													Timer3Count++;
													//60秒后还是检测到ACC OFF则MCU进入睡眠，EC20掉电关机
													if(Timer3Count == 20)
													{
														 Timer3Count = 0;
														 rdata = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1);
														 if(LOW_LEVEL != rdata){
																mcu_sleep(0);
														 }
														 else if(Timer3Count > 20)
														 {
																Timer3Count = 0;
														 } 
												  }
								  	}
										else
										{
											  Timer3Count = 0;
										}
										//若没有ACC OFF应答，则一直发ACC OFF
									  if(is_accoff_ack == FALSE)
										{
												senddatapacket(&COM2,turnoffcmd,FIXED_LENGTH+1+CHECKSUM);
										}
							}
				}
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update);			 
		 }			 
} 

void TIM4_IRQHandler(void)   
{
	  static u8 count,count1 = 0;
	  int8_t is_collision = 0;	  
	
		if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  
		{	
			  count++;
			  count1++;

			  if(sleepmode == SYSTEM_RUN && lis331dlh_Init == MEMS_SUCCESS)
				{
					 LIS331DLH_GetAccAxesRaw(&data);
					 lis331dlh_Get_Angle(&data);	
           //is_collision = lis331dlh_check_collision(1,&data);				
					 if(data.AXIS_Z<0)
 				   {
					    if(is_overturned_ack == FALSE)
						  {
							   is_overturned_ack = TRUE;
							   is_rollover_ack = TRUE;
								 Rollover[3] = FANDAO;
							   Rollover[4] = 0;
								 senddatapacket(&COM2,Rollover,FIXED_LENGTH+2+CHECKSUM);
						  }
					 }	
				   else
				   {
						  is_overturned_ack = FALSE;
				   }
				   //从0公里/小时突然加到20公里/小时（3秒内）（平时启动需要5秒钟,-103到-139这样）
				   //需要急踩油门和急刹车
				   if(data.AXIS_Y<=-200 && is_overturned_ack == FALSE && is_rollover_ack == FALSE)
				   {
						  data.AXIS_X = 0;
						  is_jiasu = TRUE;
					    accelerate[3] =2; 
						  senddatapacket(&COM2,accelerate,FIXED_LENGTH+1+CHECKSUM);
				   }
				   else
				   {
						  is_jiasu = FALSE;	
				   }							
				   if(data.AXIS_Y>=250 && is_overturned_ack == FALSE && is_rollover_ack == FALSE)
				   {
						  data.AXIS_X = 0;
						  is_jiansu = TRUE;
						  accelerate[3] =1; 
						  senddatapacket(&COM2,accelerate,FIXED_LENGTH+1+CHECKSUM);
				   }
				   else
				   {
						  is_jiansu = FALSE;	
				   }	
				}
  			if(count == 10)
				{
					  count = 0;
						GetBatteryVolt(ADC_Channel_12);
        }
        if(count1 == 3)
				{
					  count1 = 0;
					  GetBatteryVolt(ADC_Channel_10);
				}
				TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  	
		}
}

//内核复位
void MCU_NVIC_CoreReset(void)
{
   __DSB();
   SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)     |

                 (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |

                 SCB_AIRCR_VECTRESET_Msk);

   __DSB();
   while(1);
}
/*
定时器5中断处理
跳转到bootloader进行远程升级
*/
void TIM5_IRQHandler(void)   
{
		if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)  
		{	
			  TIM_Cmd(TIM5, DISABLE);
			  TIM_ClearITPendingBit(TIM5, TIM_IT_Update); 
			  DelayMS(2000);
				MCU_NVIC_CoreReset();
		}
}






