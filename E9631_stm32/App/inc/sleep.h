/*
**********************************************************************************************************
*  Copyright (C), 2009-2013, 合众思壮西安研发中心
*
*  项目名称： xxxx
*  
*  文件名称:  xxxx.h
*
*  文件描述： xxxx
*             
*             
*  创 建 者: 
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
**********************************************************************************************************
*/
#ifndef __SLEEP_H
#define __SLEEP_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "stm32f10x.h"


/*
*********************************************************************
*  全局宏定义
*********************************************************************
*/
 



/*
*********************************************************************
*  类、结构体类型定义
*********************************************************************
*/
typedef enum
{
		SYSTEM_RUN, 
		SYSTEM_SLEEP,
}SLEEP_T;

/*
*********************************************************************
*  外部引用变量声明
*********************************************************************
*/

extern SLEEP_T sleepmode;

/*
*********************************************************************
*  外部引用函数声明
*********************************************************************
*/
void mcu_sleep(void);
void mcu_makeup(void);

#ifdef __cplusplus
}
#endif

#endif /* __SLEEP__H */

