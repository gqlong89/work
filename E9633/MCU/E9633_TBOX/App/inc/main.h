/*
**********************************************************************************************************
*  Copyright (C), 2009-2012, 合众思壮西安研发中心
*
*  项目名称： E6202
*  
*  文件名称:  main.h
*
*  文件描述： 主函数头文件
*             
*             
*  创 建 者: 皇海明
*
*  创建日期：2014-05-29 
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif
	
#ifdef  MAIN_GLOBALS
#define MAIN_EXTERN
#else
#define MAIN_EXTERN   extern
#endif
	
#include "bsp.h"

/*
*********************************************************************
*  全局宏定义
*********************************************************************
*/



#define CANCOM          (&COM3)


/*
*********************************************************************
*  类、结构体类型定义
*********************************************************************
*/


/*
*********************************************************************
*  外部引用变量声明
*********************************************************************
*/

/*
*********************************************************************
*  外部引用函数声明
*********************************************************************
*/



#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

