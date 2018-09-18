/*
********************************************************************************
*  Copyright (C), 2009-2013, 合众思壮西安研发中心
*
*  项目名称：xxxx
*  
*  文件名称: xxxx.c
*
*  文件描述：xxxx
*             
*             
*  创 建 者: 皇海明
*
*  创建日期：2013-03-18 
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
#include "shell.h"
#include "GPIO.h"
#include "UartProcess.h"

extern CHARGSTATUS chargstatus;
/*
********************************************************************************
*  函数名称: GPIO_Configuration
*
*  功能描述: gpio 初始化
*
*  输入参数: 无
*
*  输出参数: 无
*
*  返 回 值: 无
*
********************************************************************************
*/
void GPIO_Configuration(void) 
{   

    GPIO_InitTypeDef GPIO_InitStructure;
   
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
	

		//BAT_DET 内置电池电压检测
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	  //ACC_DET 低电平表示ACC ON ，高电平表示ACC OFF
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	  //EXTV_DET 汽车电瓶检测（或外置电源） 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	  //STATUS_MCU  4G模块工作状态监测
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);
		
    //4G_WAKE_MCU   4G模块唤醒MCU
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		//DTR_MCU  DTE请求发送数据
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		//使能G-SENSOR
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_SetBits(GPIOA, GPIO_Pin_6);

	  //ACC ON
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOA, GPIO_Pin_11);
	
	 	//4G ENABLE  需要置高电平才能开启EC20
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		//GPIO_ResetBits(GPIOA, GPIO_Pin_12);	

   	//4G_ONOFF  4G模块/开关控制  需要置低电平才能开启EC20
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOB, &GPIO_InitStructure);
		//GPIO_ResetBits(GPIOB, GPIO_Pin_0);	

		//4G_RST 4G模块复位控制   需要置低电平才能开启EC20
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOC, GPIO_Pin_5);
		
		//MCU_WAKE_4G MCU唤醒4G模块
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		
		//硬件看门狗
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_SetBits(GPIOC, GPIO_Pin_4);
		
	  //PC4和PB14都为低电平时，关掉看门狗
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_SetBits(GPIOB, GPIO_Pin_14);
		
	  //5V EN输出高电平  4G模块、USB VBUS/RS485正常工作
   	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_SetBits(GPIOC, GPIO_Pin_12);

    //CAN1_STB
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOC, GPIO_Pin_7);
		
		//CAN2_STB
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOC, GPIO_Pin_8);

		//CHG_EN
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOD, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOD, GPIO_Pin_2);
		chargstatus = UNCHARGED;
		//USB_EN
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOC, GPIO_Pin_9);
		/*
		(PA8)输出引脚  RS485使能引脚，默认在接收状态
		PA8的高电平实现接收/发送，低电平为接收状态，高电平为发送状态
		*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);
		
		//放在此处，否则EC20开不了机
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable , ENABLE);
	  //IN3_DET(PB15)  输入引脚  默认高电平
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
		//DIG1_EN(PA14)  输入引脚 默认低电平
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOA, GPIO_Pin_14);
		
		//DIG1_EN(PA13)  输出引脚 默认低电平
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOA, GPIO_Pin_13);
}



