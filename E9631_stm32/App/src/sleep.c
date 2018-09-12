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
*  创 建 者: 韦哲轩
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
#include "sleep.h"
#include "stm32f10x_pwr.h"


SLEEP_T sleepmode;

//extern void AppInit(void);

#define RCC_PLLSource_HSE_Div1           ((u32)0x00010000)

/**********************************************************************
* 名称:RCC_Configuration()
* 功能:配置时钟
* 入口参数: 
* 出口参数:
-----------------------------------------------------------------------
* 说明:
***********************************************************************/
void RCC_Configuration(void)
{
    ErrorStatus HSEStartUpStatus;

    //使能外部晶振
    RCC_HSEConfig(RCC_HSE_ON);
    //等待外部晶振稳定
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    //如果外部晶振启动成功，则进行下一步操作
    if(HSEStartUpStatus==SUCCESS)
    {
        //设置HCLK(AHB时钟)=SYSCLK
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        //PCLK1(APB1) = HCLK/2
        RCC_PCLK1Config(RCC_HCLK_Div2);

        //PCLK2(APB2) = HCLK
        RCC_PCLK2Config(RCC_HCLK_Div1);
        
 
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_4);
        //启动PLL
        RCC_PLLCmd(ENABLE);
        //等待PLL稳定
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
        //系统时钟SYSCLK来自PLL输出
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        //切换时钟后等待系统时钟稳定
        while(RCC_GetSYSCLKSource()!=0x08);  
     }
	  /* RCC system reset(for debug purpose) */ 
}



//唤醒
void mcu_makeup(void)
{
		RCC_Configuration();
		//AppInit();
		RTU_DEBUG("mcu_makeup\r\n");
		sleepmode = SYSTEM_RUN;
}

void mcu_sleep(void)
{
	  sleepmode = SYSTEM_SLEEP;
		//进入停止模式(低功耗)，直至外部中断触发时被唤醒
		PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);
}







