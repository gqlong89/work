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
#include "Delay.h"
#include "Watchdog.h"

//硬件看门狗喂狗
void Feeddog()
{
		GPIO_SetBits(GPIOC, GPIO_Pin_4);
	  DelayMS(1);
	  GPIO_ResetBits(GPIOC, GPIO_Pin_4);
}








